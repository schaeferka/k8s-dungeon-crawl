#!/bin/bash

PROJECT_NAME="dungeon-master"
IMAGE_NAME="controller:latest"
NAMESPACE="dungeon-master-system"
MONSTERS_NAMESPACE="monsters"
MONSTIES_NAMESPACE="monsties"
CRD_FILE="./controller/config/crd/bases/kaschaefer.com_monsters.yaml"

check_namespace() {
  if kubectl get namespace "$1" >/dev/null 2>&1; then
    node ./scripts/log.js info "Namespace $1 exists."
  else
    node ./scripts/log.js info "Namespace $1 does not exist. Creating it..."
    kubectl_output=$(kubectl create namespace "$1" 2>&1)
    node ./scripts/log.js info "$kubectl_output"
  fi
}

kubectl_output=$(kubectl apply -f ./controller/config/rbac/configmap_role_binding.yaml 2>&1)
node ./scripts/log.js info "$kubectl_output"
kubectl_output=$(kubectl apply -f ./controller/config/rbac/configmap_role.yaml 2>&1)
node ./scripts/log.js info "$kubectl_output"

# Step 1: Generate CRD manifest
node ./scripts/log.js info "Generating CRD manifest..."
cd ./controller || exit
make_output=$(make manifests 2>&1)
while IFS= read -r line; do
    node ../scripts/log.js info "$line"
done <<< "$make_output"
cd ..

# Step 2: Deploy the CRD
node ./scripts/log.js info "Deploying CRD..."
if [[ -f "$CRD_FILE" ]]; then
  kubectl_output=$(kubectl apply -f "$CRD_FILE" 2>&1)
  node ./scripts/log.js info "$kubectl_output"
else
  node ./scripts/log.js error "CRD file $CRD_FILE not found. Exiting."
  exit 1
fi

# Step 3: Verify CRD deployment
node ./scripts/log.js info "Verifying CRD deployment..."
if kubectl get crds | grep -q "monsters.kaschaefer.com"; then
  node ./scripts/log.js info "CRD monsters.kaschaefer.com successfully deployed."
else
  node ./scripts/log.js error "CRD monsters.kaschaefer.com not found. Exiting."
  exit 1
fi

# Step 4: Ensure namespace exists
check_namespace "$NAMESPACE"
check_namespace "$MONSTERS_NAMESPACE"
check_namespace "$MONSTIES_NAMESPACE"

# Step 5: Build the local controller image
node ./scripts/log.js info "Building local controller image..."
make_output=$(make docker-build IMG="$IMAGE_NAME" 2>&1)
while IFS= read -r line; do
    node ./scripts/log.js info "$line"
done <<< "$make_output"

node ./scripts/log.js info "Local controller image built: $IMAGE_NAME"

# Step 6: Import the image into the k3d cluster
node ./scripts/log.js info "Starting to import Docker image '$IMAGE_NAME' into k3d cluster 'k8s-dungeon-crawl'..."

k3d_output=$(k3d image import "$IMAGE_NAME" -c k8s-dungeon-crawl 2>&1)
while IFS= read -r line; do
    node ./scripts/log.js info "$line"
done <<< "$k3d_output"

node ./scripts/log.js info "Docker image '$IMAGE_NAME' imported successfully into the k3d cluster 'k8s-dungeon-crawl'."

# Step 7: Deploy the controller using the local image
node ./scripts/log.js info "Deploying the controller with the local image..."
cd controller || exit
make_output=$(make deploy IMG="$IMAGE_NAME" 2>&1)
while IFS= read -r line; do
    node ../scripts/log.js info "$line"
done <<< "$make_output"
cd ..

# Step 8: Verify controller deployment
node ./scripts/log.js info "Verifying controller deployment..."

# Check if the controller pod exists
if kubectl get pods -n "$NAMESPACE" | grep -q "controller-manager"; then
  node ./scripts/log.js info "Controller pod found. Waiting for it to be ready..."

  # Wait for the controller pod to be ready
  kubectl_output=$(kubectl wait --for=condition=ready pod -l control-plane=controller-manager,app=controller-manager  -n "$NAMESPACE" --timeout=120s 2>&1)
  node ./scripts/log.js info "$kubectl_output"

  # Check if the kubectl wait command was successful
  if [ $? -eq 0 ]; then
    node ./scripts/log.js info "Controller pod is ready."
  else
    node ./scripts/log.js error "Controller pod did not become ready within the timeout period."
    exit 1
  fi
else
  node ./scripts/log.js error "Controller pod not found. Check the deployment."
  exit 1
fi

kubectl port-forward service/traefik -n kube-system 8080:80 > port-forward.log 2>&1 &

helm repo add prometheus-community https://prometheus-community.github.io/helm-charts > /dev/null 2>&1
helm repo update > /dev/null 2>&1
helm install prometheus prometheus-community/kube-prometheus-stack --namespace prometheus > /dev/null 2>&1

node ./scripts/log.js info "CRD and Controller deployment complete!"