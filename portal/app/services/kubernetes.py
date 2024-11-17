from kubernetes import client, config
import logging

# Configure logging
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

class KubernetesService:
    def __init__(self):
        """
        Initialize the Kubernetes client.
        Assumes the app is running inside a Kubernetes cluster or has access to a kubeconfig file.
        """
        try:
            config.load_incluster_config()  # Load config from the cluster if running inside K8s
        except config.ConfigException:
            config.load_kube_config()  # Fallback to kubeconfig for local development

        self.api = client.CustomObjectsApi()

    def create_monster_resource(self, name: str, namespace: str, monster_data: dict):
        """
        Create a Monster custom resource.
        :param name: Name of the Monster resource.
        :param namespace: Kubernetes namespace to create the resource in.
        :param monster_data: Dictionary containing monster-specific data (e.g., type, health).
        """
        monster_manifest = {
            "apiVersion": "kaschaefer.com/v1",
            "kind": "Monster",
            "metadata": {"name": name, "namespace": namespace},
            "spec": monster_data,
        }

        try:
            # Attempt to create the custom object in Kubernetes
            self.api.create_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                body=monster_manifest,
            )
            logging.info(f"Created Monster resource: {name}")
        except client.exceptions.ApiException as e:
            logging.error(f"Failed to create Monster resource: {e}")
            raise Exception(f"Failed to create Monster resource: {e}") from e

    def update_monster_resource(self, name: str, namespace: str, updates: dict):
        """
        Update an existing Monster custom resource.
        :param name: Name of the Monster resource.
        :param namespace: Kubernetes namespace containing the resource.
        :param updates: Dictionary containing the fields to update in the spec.
        """
        try:
            # Fetch the current resource
            current_resource = self.api.get_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
            )

            # Apply updates to the spec
            current_resource["spec"].update(updates)

            # Replace the resource
            self.api.replace_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
                body=current_resource,
            )
            logger.info(f"Updated Monster resource: {name}")
        except client.exceptions.ApiException as e:
            logger.error(f"Failed to update Monster resource: {e}")
            raise e

    def delete_monster_resource(self, name: str, namespace: str):
        """
        Delete a Monster custom resource.
        :param name: Name of the Monster resource.
        :param namespace: Kubernetes namespace containing the resource.
        """
        try:
            self.api.delete_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
            )
            logger.info(f"Deleted Monster resource: {name}")
        except client.exceptions.ApiException as e:
            logger.error(f"Failed to delete Monster resource: {e}")
            raise e

    def list_monsters_in_namespace(self, namespace: str):
        """
        List all Monster custom resources in a namespace.
        :param namespace: Kubernetes namespace to query.
        :return: List of Monster resources.
        """
        try:
            monsters = self.api.list_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
            )
            return monsters.get("items", [])
        except client.exceptions.ApiException as e:
            logger.error(f"Failed to list Monster resources: {e}")
            return []

    def get_monster(self, name: str, namespace: str):
        """
        Retrieve a specific Monster resource.
        :param name: Name of the Monster resource.
        :param namespace: Kubernetes namespace containing the resource.
        :return: Monster resource object or None if not found.
        """
        try:
            return self.api.get_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
            )
        except client.exceptions.ApiException as e:
            if e.status == 404:
                logger.warning(f"Monster resource not found: {name}")
                return None
            logger.error(f"Failed to retrieve Monster resource: {e}")
            raise e
