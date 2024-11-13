#!/bin/bash

# Set variables
PROJECT_NAME="dungeon-master"
IMAGE_NAME="dungeon-master:latest"
REGISTRY="<your-registry>" # Replace with your container registry (e.g., docker.io/username)
NAMESPACE="dungeon-master"
CRD_FILE="config/crd/bases/kaschaefer.com_monsters.yaml"

# Function to check if a namespace exists
check_namespace() {
  if kubectl get namespace "$1" >/dev/null 2>&1; then
    echo "Namespace $1 exists."
  else
    echo "Namespace $1 does not exist. Creating it..."
    kubectl create namespace "$1"
  fi
}

# Step 1: Generate CRD manifest
echo "Generating CRD manifest..."
make manifests

# Step 2: Deploy the CRD
echo "Deploying CRD..."
if [[ -f "$CRD_FILE" ]]; then
  kubectl apply -f "$CRD_FILE"
else
  echo "CRD file $CRD_FILE not found. Exiting."
  exit 1
fi

# Step 3: Verify CRD deployment
echo "Verifying CRD deployment..."
if kubectl get crds | grep -q "monsters.kaschaefer.com"; then
  echo "CRD monsters.kaschaefer.com successfully deployed."
else
  echo "CRD monsters.kaschaefer.com not found. Exiting."
  exit 1
fi

# Step 4: Ensure namespace exists
check_namespace "$NAMESPACE"

# Step 5: Build and push controller image
echo "Building and pushing controller image..."
make docker-build docker-push IMG="$REGISTRY/$IMAGE_NAME"

# Step 6: Deploy the controller
echo "Deploying the controller..."
make deploy IMG="$REGISTRY/$IMAGE_NAME"

# Step 7: Verify controller deployment
echo "Verifying controller deployment..."
if kubectl get pods -n "$NAMESPACE" | grep -q "controller-manager"; then
  echo "Controller successfully deployed."
else
  echo "Controller deployment failed. Check logs for more details."
  exit 1
fi

# Step 8: Create an example Monster resource
EXAMPLE_MONSTER="example-monster.yaml"
cat <<EOF > "$EXAMPLE_MONSTER"
apiVersion: kaschaefer.com/v1
kind: Monster
metadata:
  name: goblin
  namespace: $NAMESPACE
spec:
  name: Goblin
  type: goblin
  id: 10001
  health: 100
EOF

echo "Creating example Monster resource..."
kubectl apply -f "$EXAMPLE_MONSTER"

# Step 9: Verify Monster resource
echo "Verifying Monster resource..."
if kubectl get monsters -n "$NAMESPACE" | grep -q "goblin"; then
  echo "Monster resource successfully created."
else
  echo "Monster resource creation failed. Check logs for more details."
  exit 1
fi

# Finish
echo "CRD and Controller deployment complete!"
