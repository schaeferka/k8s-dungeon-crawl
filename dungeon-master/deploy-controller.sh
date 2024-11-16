#!/bin/bash

# Set variables
PROJECT_NAME="dungeon-master"
IMAGE_NAME="controller:latest"  # Use local image tag
NAMESPACE="dungeon-master-system"
MONSTERS_NAMESPACE="monsters"
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

kubectl create configmap nginx-config-map --from-file=nginx.conf
kubectl apply -f config/rbac/configmap_role_binding.yaml
kubectl apply -f config/rbac/configmap_role.yaml


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
check_namespace "$MONSTERS_NAMESPACE"

# Step 5: Build the local controller image
echo "Building local controller image..."
make docker-build IMG="$IMAGE_NAME"  # Using the local image tag

echo "Local controller image built: $IMAGE_NAME"
echo "Loading the local image into the cluster..."
k3d image import controller:latest -c k3d-k3s-default
echo "Local image loaded into the cluster."

# Step 6: Deploy the controller using the local image
echo "Deploying the controller with the local image..."
make deploy IMG="$IMAGE_NAME"  # Using the local image tag

# Step 7: Verify controller deployment
echo "Verifying controller deployment..."

# Check if the controller pod exists
if kubectl get pods -n "$NAMESPACE" | grep -q "controller-manager"; then
  echo "Controller pod found. Waiting for it to be ready..."

  # Wait for the controller pod to be in Running and Ready state
  kubectl wait --for=condition=ready pod -l control-plane=controller-manager,app=controller-manager  -n "$NAMESPACE" --timeout=60s

  # Check if the kubectl wait command was successful
  if [ $? -eq 0 ]; then
    echo "Controller pod is ready."
  else
    echo "Controller pod did not become ready within the timeout period."
    exit 1
  fi
else
  echo "Controller pod not found. Check the deployment."
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
  maxhp: 100
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
