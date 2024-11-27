#!/bin/bash

source ./.env

# Check if the resource argument is provided
if [ -z "$1" ]; then
    echo "Error: No resource argument provided. Usage: ./deploy-k8s-resources.sh <resource>"
    exit 1
fi

# Set the resource argument (game, portal, controller, etc.)
RESOURCE=$1

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
    prometheus)
        NAMESPACE_FILE="$KDC_PROMETHEUS_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_PROMETHEUS_DEPLOYMENT_FILES"
        ;;
    grafana)
        NAMESPACE_FILE="$KDC_GRAFANA_NAMESPACE_FILE"
        IFS=' ' read -r -a DEPLOYMENT_FILES <<< "$KDC_GRAFANA_DEPLOYMENT_FILES"
        ;;
    *)
        echo "Error: Invalid resource specified. Valid options are: game, portal, controller, prometheus, grafana."
        exit 1
        ;;
esac

# Check if a cluster name is provided as an argument, otherwise default to the environment variable KDC_CLUSTER_NAME
CLUSTER_NAME=$2  # The second argument for the cluster name (optional)
if [ -z "$CLUSTER_NAME" ]; then
    CLUSTER_NAME="$KDC_CLUSTER_NAME"  # Default to the cluster name from the environment
fi

# Set the kubectl context to the specified cluster
echo "Switching to cluster: $CLUSTER_NAME"
kubectl config use-context "k3d-$CLUSTER_NAME"

# Apply the namespace file
echo "Applying namespace file for $RESOURCE..."
kubectl apply -f "$NAMESPACE_FILE"

# Extract the namespace name from the namespace file
NAMESPACE=$(awk '/name:/{print $2}' "$NAMESPACE_FILE")

echo "Waiting for namespace $NAMESPACE to be ready..."
until kubectl get namespace $NAMESPACE >/dev/null 2>&1; do
    echo "Waiting for namespace $NAMESPACE..."
    sleep 1
done
echo "Namespace $NAMESPACE is ready."

# For Prometheus, deploy via Helm and set up port forwarding if specified
if [[ "$RESOURCE" == "prometheus" ]]; then
    echo "Prometheus argument detected. Deploying Prometheus..."

    # Check port availability for Prometheus
    for port in $LOCAL_PORT_9090; do
        if ! check_port $port; then
            echo "Port $port is already in use."
            echo "Please free up the port."
            echo "The port is in use by the following process: \$(lsof -i :$port)"
            echo "You can free up the port by killing the process using the port with: kill -9 \$(lsof -t -i:$port)"
            exit 1
        fi
    done

    # Add Prometheus Helm repository
    echo "Adding Prometheus community Helm chart repository..."
    helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
    helm repo update
    if [[ $? -ne 0 ]]; then
        echo "Error adding/updating Prometheus Helm repository."
        exit 1
    fi

    # Install Prometheus using Helm
    echo "Installing Prometheus..."
    helm install prometheus prometheus-community/kube-prometheus-stack --namespace $NAMESPACE

    if [[ $? -ne 0 ]]; then
        echo "Error installing Prometheus."
        exit 1
    fi

    # Wait for the Prometheus pod to be created and ready
    echo "Waiting for Prometheus pod to be created..."
    
    POD_NAME=$(kubectl get pods -n $NAMESPACE -l release=prometheus -o jsonpath='{.items[0].metadata.name}' | head -n 1)


    if [ -z "$POD_NAME" ]; then
        echo "No Prometheus pod found."
        exit 1
    fi

    echo "Prometheus pod $POD_NAME created. Waiting for it to be ready..."

    until [[ $(kubectl get pod $POD_NAME -n $NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
        echo "Waiting for Prometheus pod $POD_NAME to be ready..."
        sleep 5
    done

    echo "Prometheus pod $POD_NAME is ready."

    # Make sure service is available before starting port-forwarding
    sleep 10
fi

# Apply Kubernetes resources for the selected resource if not Prometheus
if [[ "$RESOURCE" != "prometheus" ]]; then
    echo "Applying Kubernetes resources for $RESOURCE..."
    for file in "${DEPLOYMENT_FILES[@]}"; do
        kubectl apply -f "$file"
    done

    # Wait for the pod to be created
    echo "Waiting for pod to be created..."
    until kubectl get pods -n $NAMESPACE -l app=$RESOURCE -o jsonpath="{.items[0].metadata.name}" 2>/dev/null; do
        echo "Waiting for pod to be created..."
        sleep 2
    done

    POD_NAME=$(kubectl get pods -n $NAMESPACE -l app=$RESOURCE -o jsonpath="{.items[0].metadata.name}")
    echo "Pod $POD_NAME created. Waiting for it to be ready..."

    until [[ $(kubectl get pod $POD_NAME -n $NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
        echo "Waiting for pod $POD_NAME to be ready..."
        sleep 2
    done
    echo "Pod $POD_NAME is ready."
    
fi

# Start port forwarding for the selected resource
echo "Starting port forwarding for $RESOURCE..."

# Example for Prometheus port-forwarding
if [[ "$RESOURCE" == "prometheus" ]]; then
    echo "Starting port forwarding for Grafana..."
    tmux has-session -t prometheus_port_forward 2>/dev/null
    if [ $? != 0 ]; then
        tmux new-session -d -s prometheus_port_forward "kubectl port-forward service/prometheus-kube-prometheus-prometheus -n $NAMESPACE $KDC_LOCAL_PORT_9090:9090 > prometheus-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
    else
        tmux send-keys -t prometheus_port_forward "kubectl port-forward service/prometheus-kube-prometheus-prometheus -n $NAMESPACE $KDC_LOCAL_PORT_9090:9090 > prometheus-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log" C-m
    fi
    echo "Port forwarding started for $RESOURCE."
    echo "Prometheus service is available at http://localhost:$KDC_LOCAL_PORT_9090"
fi

# Example for Brogue service port-forwarding
if [[ "$RESOURCE" == "game" ]]; then
    tmux has-session -t game_port_forward 2>/dev/null
    if [ $? != 0 ]; then
        tmux new-session -d -s game_port_forward "kubectl port-forward service/game-service -n $NAMESPACE $KDC_LOCAL_PORT_8090:8090 $KDC_LOCAL_PORT_5910:5910 $KDC_LOCAL_PORT_8010:8010 > brogue-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
    else
        tmux send-keys -t game_port_forward "kubectl port-forward service/game-service -n $NAMESPACE $KDC_LOCAL_PORT_8090:8090 $KDC_LOCAL_PORT_5910:5910 $KDC_LOCAL_PORT_8010:8010 > brogue-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log" C-m
    fi
    echo "Port forwarding started for $RESOURCE."
    echo "Game service is available at http://localhost:$KDC_LOCAL_PORT_8090"  # noVNC
    echo "Game is available with VNC at http://localhost:$KDC_LOCAL_PORT_5910"
    echo "Game is available with noVNC at http://localhost:$KDC_LOCAL_PORT_8090"
fi

# Example for Portal service port-forwarding
if [[ "$RESOURCE" == "portal" ]]; then
    tmux has-session -t portal_port_forward 2>/dev/null
    if [ $? != 0 ]; then
        tmux new-session -d -s portal_port_forward "kubectl port-forward service/portal-service -n $NAMESPACE $KDC_LOCAL_PORT_5000:5000 > portal-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
    else
        tmux send-keys -t portal_port_forward "kubectl port-forward service/portal-service -n $NAMESPACE $KDC_LOCAL_PORT_5000:5000 > portal-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log" C-m
    fi
    echo "Port forwarding started for $RESOURCE."
    echo "Portal service is available at http://localhost:$KDC_LOCAL_PORT_5000"
fi

# Example for Grafana service port-forwarding
if [[ "$RESOURCE" == "grafana" ]]; then
    tmux has-session -t grafana_port_forward 2>/dev/null
    if [ $? != 0 ]; then
        tmux new-session -d -s grafana_port_forward "kubectl port-forward service/grafana-service -n $NAMESPACE $KDC_LOCAL_PORT_3000:3000 > grafana-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
    else
        tmux send-keys -t grafana_port_forward "kubectl port-forward service/grafana-service -n $NAMESPACE $KDC_LOCAL_PORT_3000:3000 > grafana-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log" C-m
    fi
    echo "Port forwarding started for $RESOURCE."
    echo "Grafana service is available at http://localhost:$KDC_LOCAL_PORT_3000"
fi