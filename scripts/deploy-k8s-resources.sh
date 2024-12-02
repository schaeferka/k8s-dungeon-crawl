#!/bin/bash

source ./.env

# Initialize variables
SILENT=false

# Parse arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --silent) SILENT=true ;;
        *) 
            if [ -z "$RESOURCE" ]; then
                RESOURCE=$1
            else
                CLUSTER_NAME=$1
            fi
            ;;
    esac
    shift
done

# Check if the resource argument is provided
if [ -z "$RESOURCE" ]; then
    node ./scripts/log.js error "No resource argument provided. Usage: ./deploy-k8s-resources.sh <resource> [--silent] [<cluster_name>]"
    exit 1
fi

# Set the appropriate namespace and deployment files based on the resource argument
case "$RESOURCE" in
    game)
        NAMESPACE_FILE="$KDC_GAME_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_GAME_DEPLOYMENT_FILES"
        ;;
    portal)
        NAMESPACE_FILE="$KDC_PORTAL_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_PORTAL_DEPLOYMENT_FILES"
        ;;
    controller)
        NAMESPACE_FILE="$KDC_CONTROLLER_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_CONTROLLER_DEPLOYMENT_FILES"
        ;;
    traefik)
        NAMESPACE_FILE="$KDC_TRAEFIK_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_TRAEFIK_DEPLOYMENT_FILES"
        ;;
    custom404)
        NAMESPACE_FILE="$KDC_CUSTOM404_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_CUSTOM404_DEPLOYMENT_FILES"
        ;;
    prometheus)
        NAMESPACE_FILE="$KDC_PROMETHEUS_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_PROMETHEUS_DEPLOYMENT_FILES"
        ;;
    grafana)
        NAMESPACE_FILE="$KDC_GRAFANA_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_GRAFANA_DEPLOYMENT_FILES"
        ;;
    *)
        node ./scripts/log.js error "Invalid resource specified. Valid options are: game, portal, controller, traefik, custom404, prometheus, grafana."
        exit 1
        ;;
esac

# Check if a cluster name is provided as an argument, otherwise default to the environment variable KDC_CLUSTER_NAME
if [ -z "$CLUSTER_NAME" ]; then
    CLUSTER_NAME="$KDC_CLUSTER_NAME"  # Default to the cluster name from the environment
fi

# Set the kubectl context to the specified cluster
node ./scripts/log.js info "Switching to cluster: $CLUSTER_NAME"
kubectl_output=$(kubectl config use-context "k3d-$CLUSTER_NAME" 2>&1)
node ./scripts/log.js info "$kubectl_output"

# Apply the namespace file
node ./scripts/log.js info "Applying namespace file for $RESOURCE..."
kubectl_output=$(kubectl apply -f "$NAMESPACE_FILE" 2>&1)
node ./scripts/log.js info "$kubectl_output"

# Extract the namespace name from the namespace file
NAMESPACE=$(awk '/name:/{print $2}' "$NAMESPACE_FILE")

node ./scripts/log.js info "Waiting for namespace $NAMESPACE to be ready..."
until kubectl get namespace $NAMESPACE >/dev/null 2>&1; do
    node ./scripts/log.js info "Waiting for namespace $NAMESPACE..."
    sleep 1
done
node ./scripts/log.js info "Namespace $NAMESPACE is ready."

# For Prometheus, deploy via Helm and set up port forwarding if specified
if [[ "$RESOURCE" == "prometheus" ]]; then
    node ./scripts/log.js info "Prometheus argument detected. Deploying Prometheus..."

    # Check port availability for Prometheus
    for port in $LOCAL_PORT_9090; do
        if ! check_port $port; then
            node ./scripts/log.js error "Port $port is already in use. Please free up the port. The port is in use by the following process: \$(lsof -i :$port). You can free up the port by killing the process using the port with: kill -9 \$(lsof -t -i:$port)"
            exit 1
        fi
    done

    # Add Prometheus Helm repository
    node ./scripts/log.js info "Adding Prometheus community Helm chart repository..."
    helm_output=$(helm repo add prometheus-community https://prometheus-community.github.io/helm-charts 2>&1)
    while IFS= read -r line; do
        [[ -n "$line" ]] && node ./scripts/log.js info "$line"
    done <<< "$helm_output"
    
    helm_output=$(helm repo update 2>&1)
    while IFS= read -r line; do
        [[ -n "$line" ]] && node ./scripts/log.js info "$line"
    done <<< "$helm_output"
    
    if [[ $? -ne 0 ]]; then
        node ./scripts/log.js error "Error adding/updating Prometheus Helm repository."
        exit 1
    fi

    # Install Prometheus using Helm
    node ./scripts/log.js info "Installing Prometheus..."
    helm_output=$(helm install prometheus prometheus-community/kube-prometheus-stack --namespace $NAMESPACE 2>&1)
    while IFS= read -r line; do
        [[ -n "$line" ]] && node ./scripts/log.js info "$line"
    done <<< "$helm_output"

    if [[ $? -ne 0 ]]; then
        node ./scripts/log.js error "Error installing Prometheus."
        exit 1
    fi

    # Wait for the Prometheus pod to be created and ready
    node ./scripts/log.js info "Waiting for Prometheus pod to be created..."
    
    POD_NAME=$(kubectl get pods -n $NAMESPACE -l release=prometheus -o jsonpath='{.items[0].metadata.name}' | head -n 1)


    if [ -z "$POD_NAME" ]; then
        node ./scripts/log.js error "No Prometheus pod found."
        exit 1
    fi

    node ./scripts/log.js info "Prometheus pod created. Waiting for it to be ready..."

    until [[ $(kubectl get pod $POD_NAME -n $NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
        node ./scripts/log.js info "Waiting for Prometheus pod to be ready..."
        sleep 5
    done

    node ./scripts/log.js info "Prometheus pod is ready."

    # Make sure service is available before starting port-forwarding
    sleep 10
fi

# Apply the Monster CRD
if [[ "$RESOURCE" == "controller" ]]; then
    # Generate CRD manifest
    node ./scripts/log.js info "Generating CRD manifest..."
    cd ./controller || exit
    make_output=$(make manifests 2>&1)
    while IFS= read -r line; do
        node ../scripts/log.js info "$line"
    done <<< "$make_output"
    cd ..

    # Deploy the CRD
    node ./scripts/log.js info "Deploying CRD..."
    if [[ -f "$CRD_FILE" ]]; then
    kubectl_output=$(kubectl apply -f "$CRD_FILE" 2>&1)
    node ./scripts/log.js info "$kubectl_output"
    else
    node ./scripts/log.js error "CRD file $CRD_FILE not found. Exiting."
    exit 1
    fi

    # Verify CRD deployment
    node ./scripts/log.js info "Verifying CRD deployment..."
    if kubectl get crds | grep -q "monsters.kaschaefer.com"; then
    node ./scripts/log.js info "CRD monsters.kaschaefer.com successfully deployed."
    else
    node ./scripts/log.js error "CRD monsters.kaschaefer.com not found. Exiting."
    exit 1
    fi

    node ./scripts/log.js info "Deploying the controller with the local image..."
    cd controller || exit
    make_output=$(make deploy IMG="$KDC_CONTROLLER_IMAGE" 2>&1)
    while IFS= read -r line; do
        node ../scripts/log.js info "$line"
    done <<< "$make_output"
    cd ..
fi

# Apply Kubernetes resources for the selected resource if not Prometheus
if [[ "$RESOURCE" != "prometheus" && "$RESOURCE" != "controller" ]]; then
    node ./scripts/log.js info "Applying Kubernetes resources for $RESOURCE..."
    for file in "${DEPLOYMENT_FILES[@]}"; do
        kubectl_output=$(kubectl apply -f "$file" 2>&1)
        while IFS= read -r line; do
            node ./scripts/log.js info "$line"
        done <<< "$kubectl_output"
    done

    # Special handling for Traefik resources
    if [[ "$RESOURCE" != "traefik" ]]; then
        node ./scripts/log.js info "Waiting for pod to be created..."
        until POD_NAME=$(kubectl get pods -n $NAMESPACE -l app=$RESOURCE -o jsonpath="{.items[0].metadata.name}" 2>/dev/null); do
            node ./scripts/log.js info "Waiting for pod to be created..."
            sleep 2
        done

        node ./scripts/log.js info "Pod created. Waiting for it to be ready..."

        until [[ $(kubectl get pod $POD_NAME -n $NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
            node ./scripts/log.js info "Waiting for pod to be ready..."
            sleep 2
        done
        node ./scripts/log.js info "Pod is ready."
    fi
fi

# Start port forwarding for the selected resource
node ./scripts/log.js info "Starting port forwarding for $RESOURCE..."

# Prometheus port-forwarding
if [[ "$RESOURCE" == "prometheus" ]]; then
    node ./scripts/log.js info "Starting port forwarding for Prometheus..."
    kubectl_output=$(kubectl port-forward service/prometheus-kube-prometheus-prometheus -n $NAMESPACE $KDC_LOCAL_PORT_9090:9090 > prometheus-port-forward.log 2>&1 &)
    node ./scripts/log.js info "Port forwarding started for $RESOURCE."
    node ./scripts/log.js info "Prometheus service is available at http://localhost:$KDC_LOCAL_PORT_9090"
fi

# Game  service port-forwarding
if [[ "$RESOURCE" == "game" ]]; then
    node ./scripts/log.js info "Starting port forwarding for Game..."
    kubectl_output=$(kubectl port-forward service/game-service -n $NAMESPACE $KDC_LOCAL_PORT_6080:6080 $KDC_LOCAL_PORT_5910:5910 $KDC_LOCAL_PORT_8010:8010 > brogue-port-forward.log 2>&1 &)
    node ./scripts/log.js info "Port forwarding started for $RESOURCE."
    node ./scripts/log.js info "Game is available with VNC at http://localhost:$KDC_LOCAL_PORT_5910"
    node ./scripts/log.js info "Game is available with noVNC at http://localhost:$KDC_LOCAL_PORT_6080"
fi

# Portal service port-forwarding
if [[ "$RESOURCE" == "portal" ]]; then
    node ./scripts/log.js info "Starting port forwarding for Portal..."
    kubectl_output=$(kubectl port-forward service/portal-service -n $NAMESPACE $KDC_LOCAL_PORT_5000:5000 > portal-port-forward.log 2>&1 &)
    node ./scripts/log.js info "Port forwarding started for $RESOURCE."
    node ./scripts/log.js info "Portal service is available at http://localhost:$KDC_LOCAL_PORT_5000"
fi

# Grafana service port-forwarding
if [[ "$RESOURCE" == "grafana" ]]; then
    node ./scripts/log.js info "Starting port forwarding for Grafana..."
    kubectl_output=$(kubectl port-forward service/grafana-service -n $NAMESPACE $KDC_LOCAL_PORT_3000:3000 > grafana-port-forward.log 2>&1 &)
    node ./scripts/log.js info "Port forwarding started for $RESOURCE."
    node ./scripts/log.js info "Grafana service is available at http://localhost:$KDC_LOCAL_PORT_3000"
fi

# Traefik service port-forwarding
if [[ "$RESOURCE" == "traefik" ]]; then
    node ./scripts/log.js info "Starting port forwarding for Traefik/controller..."
    kubectl_output=$(kubectl port-forward service/traefik -n kube-system $KDC_LOCAL_PORT_8080:80 > traefik-port-forward.log 2>&1 &)
    node ./scripts/log.js info "Port forwarding started for $RESOURCE."
    node ./scripts/log.js info "Traefik service is available at http://localhost:$KDC_LOCAL_PORT_8080"
fi

# Suppress output if --silent flag is set
if [ "$SILENT" = true ]; then
    exec > /dev/null 2>&1
fi