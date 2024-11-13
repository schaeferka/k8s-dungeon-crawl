#!/bin/bash

# This script automates the deployment of Brogue, Portal, DM, Prometheus, and Grafana
# on a local Kubernetes cluster using k3d. It builds the Docker images for Brogue
# and Portal, starts the k3d cluster, imports the Docker images into the cluster,
# and applies the Kubernetes resources for Brogue and Portal. It also sets up port
# forwarding for Brogue and Portal so that they can be accessed locally.
#
# The script also supports the following arguments:
# - build: Build the Docker images for Brogue and Portal
# - prom: Reset the Prometheus deployment
# - grafana: Reset the Grafana deployment


# Set directory variables
SCRIPT_DIR=$(dirname "$0")
BROGUE_DIR="$SCRIPT_DIR/brogue-game"
PORTAL_DIR="$SCRIPT_DIR/portal"
DM_DIR="$SCRIPT_DIR/dungeon-master"
GRAFANA_DIR="$SCRIPT_DIR/grafana"
PROMETHEUS_DIR="$SCRIPT_DIR/prometheus"
IMAGE_NAME_BROGUE="broguek8s:latest"
IMAGE_NAME_PORTAL="portalk8s:latest"
IMAGE_NAME_PROMETHEUS="bitnami/prometheus:latest"
IMAGE_NAME_GRAFANA="grafana/grafana:latest"
CLUSTER_NAME="k3d-k3s-default"
BROGUE_NAMESPACE_FILE="$BROGUE_DIR/k8s/brogue-namespace.yaml"
PORTAL_NAMESPACE_FILE="$PORTAL_DIR/k8s/portal-namespace.yaml"
DM_NAMESPACE_FILE="$DM_DIR/k8s/dungeon-master-namespace.yaml"
PROMETHEUS_NAMESPACE_FILE="$PROMETHEUS_DIR/k8s/prometheus-namespace.yaml"
GRAFANA_NAMESPACE_FILE="$GRAFANA_DIR/k8s/grafana-namespace.yaml"
DEPLOYMENT_FILES_BROGUE=("$BROGUE_DIR/k8s/brogue-deployment.yaml" "$BROGUE_DIR/k8s/brogue-service.yaml" "$BROGUE_DIR/k8s/brogue-clusterroles.yaml" "$BROGUE_DIR/k8s/brogue-clusterrolebindings.yaml" "$BROGUE_DIR/k8s/brogue-serviceaccount.yaml")
DEPLOYMENT_FILES_PORTAL=("$PORTAL_DIR/k8s/portal-deployment.yaml" "$PORTAL_DIR/k8s/portal-service.yaml" "$PORTAL_DIR/k8s/portal-clusterrole.yaml" "$PORTAL_DIR/k8s/portal-clusterrolebinding.yaml" "$PORTAL_DIR/k8s/portal-serviceaccount.yaml")    
DEPLOYMENT_FILES_DM=("$DM_DIR/k8s/monster-crd.yaml" "$DM_DIR/k8s/goblin.yaml")
DEPLOYMENT_FILES_PROMETHEUS=("$PROMETHEUS_DIR/k8s/prometheus-deploy.yaml" "$PROMETHEUS_DIR/k8s/prometheus-service.yaml" "$PROMETHEUS_DIR/k8s/prometheus-config.yaml")
DEPLOYMENT_FILES_GRAFANA=("$GRAFANA_DIR/k8s/grafana-datasources-config.yaml" "$GRAFANA_DIR/k8s/grafana-dashboard-config.yaml" "$GRAFANA_DIR/k8s/grafana-dashboard-provisioning-config.yaml" "$GRAFANA_DIR/k8s/grafana-deploy.yaml" "$GRAFANA_DIR/k8s/grafana-service.yaml")
LOCAL_PORT_16080=8090  # noVNC server port for Brogue
LOCAL_PORT_15900=5910  # VNC server port for Brogue
LOCAL_PORT_18000=8010  # Metrics server port for Brogue
LOCAL_PORT_5000=5001   # Portal server port
LOCAL_PORT_3000=3000   # Grafana server port
LOCAL_PORT_9090=9090   # Prometheus server port

# Function to check if a port is in use
check_port() {
    local port=$1
    if lsof -i ":$port" &>/dev/null; then
        return 1  # Port is in use
    else
        return 0  # Port is available
    fi
}


# Check each port individually and prompt for action if any are in use
for port in $LOCAL_PORT_16080 $LOCAL_PORT_15900 $LOCAL_PORT_18000 $LOCAL_PORT_5000; do
    if ! check_port $port; then
        echo "Port $port is already in use."
        echo "Please free up the port."
        echo "The port is in use by the following process: \$(lsof -i :$port)"
        echo "You can free up the port by killing the process using the port with: kill -9 \$(lsof -t -i:$port)"
        exit 1
    fi
done

# Step 1: Build Docker images
if [[ " $@ " =~ " build " ]]; then
    echo "Building Docker images for Brogue and Portal..."
    docker build -t $IMAGE_NAME_BROGUE $BROGUE_DIR
    docker build -t $IMAGE_NAME_PORTAL $PORTAL_DIR
else
    echo "Skipping Docker image build."
fi

# Step 2: Start the k3d cluster if it's not already running
echo "Checking if k3d cluster $CLUSTER_NAME is running..."
if ! k3d cluster list | grep -q "^$CLUSTER_NAME"; then
    echo "Cluster $CLUSTER_NAME is not running. Starting cluster..."
    k3d cluster create $CLUSTER_NAME --port "$LOCAL_PORT_16080:30080@loadbalancer" --port "$LOCAL_PORT_15900:30090@loadbalancer" --port "$LOCAL_PORT_18000:30800@loadbalancer" --port "$LOCAL_PORT_5000:30500@loadbalancer"
else
    echo "Cluster $CLUSTER_NAME is already running."
fi

# Step 3: Import Docker images into k3d cluster
echo "Importing Docker images into k3d cluster..."
k3d image import $IMAGE_NAME_BROGUE -c $CLUSTER_NAME
k3d image import $IMAGE_NAME_PORTAL -c $CLUSTER_NAME

# Step 4: Delete existing resources in the cluster for both Brogue and Portal
echo "Deleting existing Kubernetes resources for Brogue and Portal..."
for file in "${DEPLOYMENT_FILES_BROGUE[@]}" "${DEPLOYMENT_FILES_PORTAL[@]}"; do
    kubectl delete -f "$file" --ignore-not-found
done
kubectl delete -f "$BROGUE_NAMESPACE_FILE" --ignore-not-found
kubectl delete -f "$PORTAL_NAMESPACE_FILE" --ignore-not-found

# Step 5: Apply the namespace YAML files
echo "Applying Brogue, Portal, DM namespace files..."
kubectl apply -f "$BROGUE_NAMESPACE_FILE"
kubectl apply -f "$PORTAL_NAMESPACE_FILE"
kubectl apply -f "$DM_NAMESPACE_FILE"

# Wait for the namespaces to be ready
BROGUE_NAMESPACE=$(awk '/name:/{print $2}' "$BROGUE_NAMESPACE_FILE")
PORTAL_NAMESPACE=$(awk '/name:/{print $2}' "$PORTAL_NAMESPACE_FILE")
DM_NAMESPACE=$(awk '/name:/{print $2}' "$DM_NAMESPACE_FILE")
for NAMESPACE in $BROGUE_NAMESPACE $PORTAL_NAMESPACE $DM_NAMESPACE; do
    echo "Waiting for namespace $NAMESPACE to be ready..."
    until kubectl get namespace $NAMESPACE >/dev/null 2>&1; do
        echo "Waiting for namespace $NAMESPACE..."
        sleep 1
    done
    echo "Namespace $NAMESPACE is ready."
done

# Step 6: Apply the other YAML files for Brogue, Portal and DM
echo "Applying Brogue, Portal, DM Kubernetes resources..."
for file in "${DEPLOYMENT_FILES_BROGUE[@]}" "${DEPLOYMENT_FILES_PORTAL[@]}" "${DEPLOYMENT_FILES_DM[@]}"; do
    kubectl apply -f "$file"
done

# Step 7: Wait for the Brogue, Portal, and DM pods to be created and ready

# Wait for Brogue pod to be created and ready
echo "Waiting for Brogue pod to be created..."
until kubectl get pods -n $BROGUE_NAMESPACE -l app=brogue -o jsonpath="{.items[0].metadata.name}" 2>/dev/null; do
    echo "Waiting for Brogue pod to be created..."
    sleep 2
done

BROGUE_POD_NAME=$(kubectl get pods -n $BROGUE_NAMESPACE -l app=brogue -o jsonpath="{.items[0].metadata.name}")
echo "Brogue pod $BROGUE_POD_NAME created. Waiting for it to be ready..."

until [[ $(kubectl get pod $BROGUE_POD_NAME -n $BROGUE_NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
    echo "Waiting for Brogue pod $BROGUE_POD_NAME to be ready..."
    sleep 2
done
echo "Brogue pod $BROGUE_POD_NAME is ready."

# Wait for Portal pod to be created and ready
echo "Waiting for Portal pod to be created..."
until kubectl get pods -n $PORTAL_NAMESPACE -l app=portal -o jsonpath="{.items[0].metadata.name}" 2>/dev/null; do
    echo "Waiting for Portal pod to be created..."
    sleep 2
done

PORTAL_POD_NAME=$(kubectl get pods -n $PORTAL_NAMESPACE -l app=portal -o jsonpath="{.items[0].metadata.name}")
echo "Portal pod $PORTAL_POD_NAME created. Waiting for it to be ready..."

until [[ $(kubectl get pod $PORTAL_POD_NAME -n $PORTAL_NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
    echo "Waiting for Portal pod $PORTAL_POD_NAME to be ready..."
    sleep 2
done
echo "Portal pod $PORTAL_POD_NAME is ready."


# Step 7: Reset the Prometheus deployment if prom in command line
if [[ " $@ " =~ " prom " ]]; then
    echo "Prometheus argument detected. Resetting Prometheus deployment..."

    # Check each port individually and prompt for action if any are in use
    for port in $LOCAL_PORT_9090; do
        if ! check_port $port; then
            echo "Port $port is already in use."
            echo "Please free up the port."
            echo "The port is in use by the following process: \$(lsof -i :$port)"
            echo "You can free up the port by killing the process using the port with: kill -9 \$(lsof -t -i:$port)"
            exit 1
        fi
    done

    # Reset the Prometheus namespace
    echo "Deleting existing Kubernetes resources for Prometheus..."
    for file in "${DEPLOYMENT_FILES_PROMETHEUS[@]}"; do
        kubectl delete -f "$file" --ignore-not-found
    done
    kubectl delete -f "$PROMETHEUS_NAMESPACE_FILE" --ignore-not-found

    # Apply the Prometheus namespace YAML file
    echo "Applying Prometheus namespace file..."
    kubectl apply -f "$PROMETHEUS_NAMESPACE_FILE"

    # Wait for the namespace to be ready
    PROMETHEUS_NAMESPACE=$(awk '/name:/{print $2}' "$PROMETHEUS_NAMESPACE_FILE")
    echo "Waiting for namespace $PROMETHEUS_NAMESPACE to be ready..."
    until kubectl get namespace $PROMETHEUS_NAMESPACE >/dev/null 2>&1; do
        echo "Waiting for namespace $PROMETHEUS_NAMESPACE..."
        sleep 1
    done
    echo "Namespace $PROMETHEUS_NAMESPACE is ready."

    echo "Applying Prometheus Kubernetes resources..."
    for file in "${DEPLOYMENT_FILES_PROMETHEUS[@]}"; do
        kubectl apply -f "$file"
    done

    # Wait for Prometheus pod to be created and ready
    echo "Waiting for Prometheus pod to be created..."
    until kubectl get pods -n $PROMETHEUS_NAMESPACE -l app=prometheus -o jsonpath="{.items[0].metadata.name}" 2>/dev/null; do
        echo "Waiting for Prometheus pod to be created..."
        sleep 2
    done

    PROMETHEUS_POD_NAME=$(kubectl get pods -n $PROMETHEUS_NAMESPACE -l app=prometheus -o jsonpath="{.items[0].metadata.name}")
    echo "Prometheus pod $PROMETHEUS_POD_NAME created. Waiting for it to be ready..."

    until [[ $(kubectl get pod $PROMETHEUS_POD_NAME -n $PROMETHEUS_NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
        echo "Waiting for Prometheus pod $PROMETHEUS_POD_NAME to be ready..."
        sleep 2
    done
    echo "Prometheus pod $PROMETHEUS_POD_NAME is ready."
fi

# Step 8: Reset the Grafana deployment if grafana in command line
if [[ " $@ " =~ " grafana " ]]; then
    echo "Grafana argument detected. Resetting Grafana deployment..."

    # Check each port individually and prompt for action if any are in use
        for port in $LOCAL_PORT_3000; do
            if ! check_port $port; then
                echo "Port $port is already in use."
                echo "Please free up the port."
                echo "The port is in use by the following process: \$(lsof -i :$port)"
                echo "You can free up the port by killing the process using the port with: kill -9 \$(lsof -t -i:$port)"
                exit 1
            fi
        done
        # Reset the Grafana namespace
        echo "Deleting existing Kubernetes resources for Grafana..."
        for file in "${DEPLOYMENT_FILES_GRAFANA[@]}"; do
            kubectl delete -f "$file" --ignore-not-found
        done
        kubectl delete -f "$GRAFANA_NAMESPACE_FILE" --ignore-not-found

        # Apply the Grafana namespace YAML file
        echo "Applying Grafana namespace file..."
        kubectl apply -f "$GRAFANA_NAMESPACE_FILE"

        # Wait for the namespace to be ready
        GRAFANA_NAMESPACE=$(awk '/name:/{print $2}' "$GRAFANA_NAMESPACE_FILE")
        echo "Waiting for namespace $GRAFANA_NAMESPACE to be ready..."
        until kubectl get namespace $GRAFANA_NAMESPACE >/dev/null 2>&1; do
            echo "Waiting for namespace $GRAFANA_NAMESPACE..."
            sleep 1
        done
        echo "Namespace $GRAFANA_NAMESPACE is ready."

        echo "Applying Grafana Kubernetes resources..."
        for file in "${DEPLOYMENT_FILES_GRAFANA[@]}"; do
            kubectl apply -f "$file"
        done

        # Wait for Grafana pod to be created and ready
        echo "Waiting for Grafana pod to be created..."
        until kubectl get pods -n $GRAFANA_NAMESPACE -l app=grafana -o jsonpath="{.items[0].metadata.name}" 2>/dev/null; do
            echo "Waiting for Grafana pod to be created..."
            sleep 2
        done

        GRAFANA_POD_NAME=$(kubectl get pods -n $GRAFANA_NAMESPACE -l app=grafana -o jsonpath="{.items[0].metadata.name}")
        echo "Grafana pod $GRAFANA_POD_NAME created. Waiting for it to be ready..."

        until [[ $(kubectl get pod $GRAFANA_POD_NAME -n $GRAFANA_NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
            echo "Waiting for Grafana pod $GRAFANA_POD_NAME to be ready..."
            sleep 2
        done
        echo "Grafana pod $GRAFANA_POD_NAME is ready."
fi

# Step 9: Start port forwarding in a new tmux session
echo "Starting port forwarding in a tmux session..."
tmux new-session -d -s brogue_port_forward "kubectl port-forward service/brogue-service -n $BROGUE_NAMESPACE $LOCAL_PORT_16080:16080 $LOCAL_PORT_15900:15900 $LOCAL_PORT_18000:18000 > port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
tmux new-session -d -s portal_port_forward "kubectl port-forward service/portal-service -n $PORTAL_NAMESPACE $LOCAL_PORT_5000:5000 > portal-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
tmux new-session -d -s prometheus_port_forward "kubectl port-forward service/prometheus-service -n $PROMETHEUS_NAMESPACE $LOCAL_PORT_9090:9090 > prometheus-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"
tmux new-session -d -s grafana_port_forward "kubectl port-forward service/grafana-service -n $GRAFANA_NAMESPACE $LOCAL_PORT_3000:3000 > grafana-port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"


# Inform the user about tmux sessions
echo "Port forwarding started in tmux sessions named 'brogue_port_forward' and 'portal_port_forward'."
echo "You can attach to the sessions with: tmux attach -t brogue_port_forward or tmux attach -t portal_port_forward"
echo "Or kill the sessions with: tmux kill-session -t brogue_port_forward or tmux kill-session -t portal_port_forward"
echo
echo "Deployment complete. You can connect to Brogue using the following options:"
echo " - For noVNC, open your browser and go to: http://localhost:$LOCAL_PORT_16080"
echo " - For VNC, connect using a VNC client to: localhost:$LOCAL_PORT_15900"
echo " - For metrics, connect to: http://localhost:$LOCAL_PORT_18000/metrics"
echo " - For Prometheus, connect to: http://localhost:$LOCAL_PORT_9090"
echo " - For Grafana, connect to: http://localhost:$LOCAL_PORT_3000"
echo
echo "To access the Portal, connect to: http://localhost:$LOCAL_PORT_5000/metrics"
