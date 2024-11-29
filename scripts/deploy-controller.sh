#!/bin/bash

PROJECT_NAME="dungeon-master"
IMAGE_NAME="controller:latest"
NAMESPACE="dungeon-master-system"
MONSTERS_NAMESPACE="monsters"
MONSTIES_NAMESPACE="monsties"
CRD_FILE="./controller/config/crd/bases/kaschaefer.com_monsters.yaml"

check_namespace() {
  if kubectl get namespace "$1" >/dev/null 2>&1; then
    echo "Namespace $1 exists."
  else
    echo "Namespace $1 does not exist. Creating it..."
    kubectl create namespace "$1"
  fi
}

kubectl apply -f ./controller/config/rbac/configmap_role_binding.yaml
kubectl apply -f ./controller/config/rbac/configmap_role.yaml


# Step 1: Generate CRD manifest
echo "Generating CRD manifest..."
cd ./controller || exit
make manifests
cd ..

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
check_namespace "$MONSTIES_NAMESPACE"

# Step 5: Build the local controller image
echo "Building local controller image..."
make docker-build IMG="$IMAGE_NAME"  

echo "Local controller image built: $IMAGE_NAME"
echo "Loading the local image into the cluster..."
k3d image import controller:latest -c k8s-dungeon-crawl
echo "Local image loaded into the cluster."

# Step 6: Deploy the controller using the local image
echo "Deploying the controller with the local image..."
cd controller || exit
make deploy IMG="$IMAGE_NAME"  
cd ..

# Step 7: Verify controller deployment
echo "Verifying controller deployment..."

# Check if the controller pod exists
if kubectl get pods -n "$NAMESPACE" | grep -q "controller-manager"; then
  echo "Controller pod found. Waiting for it to be ready..."

  # Wait for the controller pod to be ready
  kubectl wait --for=condition=ready pod -l control-plane=controller-manager,app=controller-manager  -n "$NAMESPACE" --timeout=120s

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

kubectl port-forward service/traefik -n kube-system 8080:80 > port-forward.log 2>&1 &


echo "CRD and Controller deployment complete!"
