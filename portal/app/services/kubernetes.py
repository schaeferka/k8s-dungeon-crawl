from kubernetes import client, config
from flask import current_app

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
            self.api.create_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                body=monster_manifest,
            )
            current_app.logger.info(f"Created Monster resource: {name}")
        except client.exceptions.ApiException as e:
            current_app.logger.error(f"Failed to create Monster resource: {e}")
            raise Exception(f"Failed to create Monster resource: {e}") from e

    
    def update_monster_resource(self, name: str, namespace: str, monster_data: dict):
        """
        Update an existing Monster custom resource.
        :param name: Name of the Monster resource.
        :param namespace: Kubernetes namespace containing the resource.
        :param monster_data: Dictionary containing the fields to update in the spec.
        """
        try:
            current_resource = self.api.get_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
            )

            if "spec" not in current_resource:
                raise ValueError(f"Monster resource {name} does not have a 'spec' field")

            current_resource["spec"].update(monster_data)

            self.api.replace_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
                body=current_resource,
            )
            
            current_app.logger.info(f"Updated Monster resource: {name}")

        except client.exceptions.ApiException as e:
            current_app.logger.error(f"Failed to update Monster resource: {e}")
            raise e
        except ValueError as e:
            current_app.logger.error(f"Invalid resource structure: {e}")
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
            current_app.logger.info(f"Successfully deleted Monster resource: {name} in namespace {namespace}")
        except client.exceptions.ApiException as e:
            current_app.logger.error(f"Failed to delete Monster resource {name} in namespace {namespace}: {e}")
            raise e
        except Exception as e:
            current_app.logger.error(f"Unexpected error while deleting Monster resource {name}: {e}")
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
            current_app.logger.error(f"Failed to list Monster resources: {e}")
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
                current_app.logger.warning(f"Monster resource not found: {name}")
                return None
            current_app.logger.error(f"Failed to retrieve Monster resource: {e}")
            raise e

    def delete_all_monsters_in_namespace(self, namespace: str):
        """
        Delete all Monster custom resources in a namespace.
        :param namespace: Kubernetes namespace containing the resources to delete.
        """
        try:
            monsters = self.list_monsters_in_namespace(namespace)
            
            if not monsters:
                current_app.logger.info(f"No Monster resources found in namespace {namespace}")
                return
            
            for monster in monsters:
                name = monster.get("metadata", {}).get("name")
                if name:
                    self.delete_monster_resource(name, namespace)
                else:
                    current_app.logger.warning(f"Monster resource missing 'name' field: {monster}")
                    
        except client.exceptions.ApiException as e:
            current_app.logger.error(f"Failed to delete Monster resources in namespace {namespace}: {e}")
            raise e
        except Exception as e:
            current_app.logger.error(f"Unexpected error while deleting Monster resources in namespace {namespace}: {e}")
            raise e