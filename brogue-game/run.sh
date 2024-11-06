#!/bin/bash

# Set directory variables
SCRIPT_DIR=$(dirname "$0")/k8s
IMAGE_NAME="broguek8s:latest"
CLUSTER_NAME="k3d-k3s-default"
NAMESPACE_FILE="$SCRIPT_DIR/brogue-namespace.yaml"
DEPLOYMENT_FILES=("$SCRIPT_DIR/brogue-deployment.yaml" "$SCRIPT_DIR/brogue-service.yaml" "$SCRIPT_DIR/brogue-clusterroles.yaml" "$SCRIPT_DIR/brogue-clusterrolebindings.yaml" "$SCRIPT_DIR/brogue-serviceaccount.yaml")
LOCAL_PORT_16080=8090
LOCAL_PORT_15900=5910
LOCAL_PORT_18000=8010  # Add the metrics server port

# Function to check if a port is in use
check_port() {
    local port=$1
    if lsof -i ":$port" &>/dev/null; then
        return 1  # Port is in use
    else
        return 0  # Port is available
    fi
}

# Function to find the next available port starting from the given port
find_available_port() {
    local port=$1
    while check_port $port; do
        ((port++))
    done
    echo $port
}

# Check if "yolo" argument is provided
if [[ " $@ " =~ " yolo " ]]; then
    echo "YOLO mode: Automatically finding available ports for $LOCAL_PORT_16080, $LOCAL_PORT_15900, and $LOCAL_PORT_18000 if in use."
    
    # Find available port for noVNC
    LOCAL_PORT_16080=$(find_available_port $LOCAL_PORT_16080)
    echo "Selected noVNC port: $LOCAL_PORT_16080"

    # Find available port for VNC
    LOCAL_PORT_15900=$(find_available_port $LOCAL_PORT_15900)
    echo "Selected VNC port: $LOCAL_PORT_15900"

    # Find available port for Metrics
    LOCAL_PORT_18000=$(find_available_port $LOCAL_PORT_18000)
    echo "Selected Metrics port: $LOCAL_PORT_18000"
else
    # Check each port individually and prompt for action if any are in use
    if ! check_port $LOCAL_PORT_16080; then
        echo "Port $LOCAL_PORT_16080 is already in use."
        echo "To free up this port, you can identify the process using it with:"
        echo "   lsof -i :$LOCAL_PORT_16080"
        echo "Then, terminate the process by running:"
        echo "   kill -9 <PID>"
        echo "Or, alternatively, change the LOCAL_PORT_16080 variable in this script to an available port."
        exit 1
    fi

    if ! check_port $LOCAL_PORT_15900; then
        echo "Port $LOCAL_PORT_15900 is already in use."
        echo "To free up this port, you can identify the process using it with:"
        echo "   lsof -i :$LOCAL_PORT_15900"
        echo "Then, terminate the process by running:"
        echo "   kill -9 <PID>"
        echo "Or, alternatively, change the LOCAL_PORT_15900 variable in this script to an available port."
        exit 1
    fi

    if ! check_port $LOCAL_PORT_18000; then
        echo "Port $LOCAL_PORT_18000 is already in use."
        echo "To free up this port, you can identify the process using it with:"
        echo "   lsof -i :$LOCAL_PORT_18000"
        echo "Then, terminate the process by running:"
        echo "   kill -9 <PID>"
        echo "Or, alternatively, change the LOCAL_PORT_18000 variable in this script to an available port."
        exit 1
    fi
fi

# Proceed with the rest of the script if ports are available or have been reassigned in YOLO mode
echo "Ports $LOCAL_PORT_16080, $LOCAL_PORT_15900, and $LOCAL_PORT_18000 are available. Continuing with deployment..."

# Step 1: Check if "build" argument is included, and build the Docker image if so
if [[ " $@ " =~ " build " ]]; then
    echo "Building Docker image $IMAGE_NAME..."
    docker build -t $IMAGE_NAME .
else
    echo "Skipping Docker image build."
fi

# Step 2: Start the k3d cluster if it's not already running
echo "Checking if k3d cluster $CLUSTER_NAME is running..."
if ! k3d cluster list | grep -q "^$CLUSTER_NAME"; then
    echo "Cluster $CLUSTER_NAME is not running. Starting cluster..."
    k3d cluster create $CLUSTER_NAME --port "$LOCAL_PORT_16080:30080@loadbalancer" --port "$LOCAL_PORT_15900:30090@loadbalancer" --port "$LOCAL_PORT_18000:30800@loadbalancer"
else
    echo "Cluster $CLUSTER_NAME is already running."
fi

# Step 3: Import the Docker image into the k3d cluster
echo "Importing Docker image into k3d cluster..."
k3d image import $IMAGE_NAME -c $CLUSTER_NAME

# Step 4: Delete the existing resources in the cluster
echo "Deleting existing Kubernetes resources..."
for file in "${DEPLOYMENT_FILES[@]}"; do
    kubectl delete -f "$file" --ignore-not-found
done
kubectl delete -f "$NAMESPACE_FILE" --ignore-not-found

# Step 5: Apply the namespace YAML file
echo "Applying namespace file $NAMESPACE_FILE..."
kubectl apply -f "$NAMESPACE_FILE"

# Wait for the namespace to be ready
NAMESPACE=$(awk '/name:/{print $2}' "$NAMESPACE_FILE")
echo "Waiting for namespace $NAMESPACE to be ready..."
until kubectl get namespace $NAMESPACE >/dev/null 2>&1; do
    echo "Waiting for namespace $NAMESPACE..."
    sleep 1
done
echo "Namespace $NAMESPACE is ready."

# Step 6: Apply the other YAML files
echo "Applying other Kubernetes resources..."
for file in "${DEPLOYMENT_FILES[@]}"; do
    kubectl apply -f "$file"
done

# Step 7: Wait for the Brogue pod to be created
echo "Waiting for pod to be created..."
until kubectl get pods -n $NAMESPACE -l app=brogue -o jsonpath="{.items[0].metadata.name}" 2>/dev/null; do
    echo "Waiting for pod to be created..."
    sleep 2
done

# Get the pod name
POD_NAME=$(kubectl get pods -n $NAMESPACE -l app=brogue -o jsonpath="{.items[0].metadata.name}")
echo "Pod $POD_NAME created. Waiting for it to be ready..."

# Wait for the Brogue pod to be ready
until [[ $(kubectl get pod $POD_NAME -n $NAMESPACE -o jsonpath='{.status.containerStatuses[0].ready}') == "true" ]]; do
    echo "Waiting for pod $POD_NAME to be ready..."
    sleep 2
done
echo "Pod $POD_NAME is ready."

# Step 8: Start port forwarding in a new tmux session
# Wait for the service to be ready
echo "Waiting for brogue-service to be available..."
until kubectl get svc -n $NAMESPACE brogue-service >/dev/null 2>&1; do
    echo "Waiting for brogue-service to be available..."
    sleep 2
done
echo "brogue-service is available. Starting port forwarding..."

echo "Starting port forwarding in a tmux session..."
tmux new-session -d -s brogue_port_forward "kubectl port-forward service/brogue-service -n $NAMESPACE $LOCAL_PORT_16080:16080 $LOCAL_PORT_15900:15900 $LOCAL_PORT_18000:18000 > port-forward.log 2>&1 || echo 'Port-forwarding failed' > tmux-error.log"

# Inform the user about tmux session
echo "Port forwarding started in a tmux session named 'brogue_port_forward'."
echo "You can attach to the session with: tmux attach -t brogue_port_forward"
echo "Or kill the session with: tmux kill-session -t brogue_port_forward"
echo
echo "Deployment complete. You can connect to Brogue using the following options:"
echo " - For noVNC, open your browser and go to: http://localhost:$LOCAL_PORT_16080"
echo " - For VNC, connect using a VNC client to: localhost:$LOCAL_PORT_15900"
echo " - For metrics, connect to: http://localhost:$LOCAL_PORT_18000/metrics"