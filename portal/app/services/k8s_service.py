"""
This module defines the `KubernetesService` class, which provides methods for managing Monster
custom resources within a Kubernetes cluster. The class allows the creation, update, deletion,
and retrieval of Monster resources, as well as the ability to list all Monster resources in a
given namespace. It also includes functionality for bulk deletion of Monster resources.

The `KubernetesService` class interacts with the Kubernetes CustomObjects API to manage custom
resources of the `Monster` kind, using the `kaschaefer.com/v1` API version. Additionally, it
integrates with Flask's logging system to log operations and errors related to Monster resources.

Features:
- Create, update, and delete Monster resources in Kubernetes.
- List and retrieve Monster resources by name and namespace.
- Bulk delete Monster resources within a namespace.
- Integration with Prometheus for monitoring (via logging).

Prometheus integration:
- Logs success or failure of each operation, such as resource creation, update, and deletion.
- Logs any errors that occur during interactions with the Kubernetes API.

The class assumes that the application is running inside a Kubernetes cluster or has access to a
Kubeconfig file for local development.

Methods:
    - create_monster_resource: Create a new Monster custom resource.
    - update_monster_resource: Update an existing Monster custom resource.
    - delete_monster_resource: Delete a specific Monster custom resource.
    - list_monsters_in_namespace: List all Monster custom resources in a namespace.
    - get_monster: Retrieve a specific Monster custom resource by name.
    - delete_all_monsters_in_namespace: Delete all Monster resources in a namespace.
"""
import time
from flask import current_app
from kubernetes import config, client
from kubernetes.client.rest import ApiException

class KubernetesService:
    """
    Service class for interacting with Kubernetes to manage Monster custom resources.

    This class allows the creation, updating, deletion, and retrieval of Monster custom resources
    in a Kubernetes cluster. It also interacts with the Kubernetes API to manage custom objects
    and provides logging for each operation.

    Methods:
        create_monster_resource: Create a new Monster custom resource.
        update_monster_resource: Update an existing Monster custom resource.
        delete_monster_resource: Delete a specific Monster custom resource.
        list_monsters_in_namespace: List all Monster custom resources in a given namespace.
        get_monster: Retrieve a specific Monster custom resource by name.
        delete_all_monsters_in_namespace: Delete all Monster resources in a given namespace.
    """

    def __init__(self):
        """
        Initialize the Kubernetes client.

        This method attempts to load the Kubernetes configuration from within the cluster.
        If the app is running outside of a Kubernetes cluster, it falls back to loading
        the kubeconfig file for local development.

        Raises:
            config.ConfigException: If neither in-cluster config nor kubeconfig is available.
        """
        try:
            config.load_incluster_config()  # Load config from the cluster if running inside K8s
        except config.ConfigException:
            config.load_kube_config()  # Fallback to kubeconfig for local development

        self.api = client.CustomObjectsApi()

    def resource_exists(self, name: str, namespace: str) -> bool:
        """
        Check if a Monster custom resource exists.

        Args:
            name (str): The name of the Monster resource.
            namespace (str): The Kubernetes namespace where the resource is located.

        Returns:
            bool: True if the resource exists, False otherwise.
        """
        try:
            self.api.get_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
            )
            return True
        except client.exceptions.ApiException as e:
            if e.status == 404:
                return False
            raise e

    def create_monster_resource(self, name: str, namespace: str, monster_data: dict):
        """
        Create a Monster custom resource in the specified namespace.

        Args:
            name (str): The name of the Monster resource.
            namespace (str): The Kubernetes namespace where the resource will be created.
            monster_data (dict): Dictionary containing the monster-specific data (e.g., 
            type, health).

        Raises:
            Exception: If there is an error creating the Monster resource.
        """
        if self.resource_exists(name, namespace):
            current_app.logger.warning(f"Monster resource {name} already exists in namespace \
                {namespace} - unable to create")
            return

        # Convert keys in monster_data to camelCase
        monster_data = {
            "accuracy": monster_data.get("accuracy"),
            "attackSpeed": monster_data.get("attack_speed"),
            "damageMax": monster_data.get("damage_max"),
            "damageMin": monster_data.get("damage_min"),
            "deathTimestamp": monster_data.get("death_timestamp"),
            "defense": monster_data.get("defense"),
            "depth": monster_data.get("depth"),
            "hp": monster_data.get("hp"),
            "id": monster_data.get("id"),
            "isDead": monster_data.get("is_dead"),
            "maxHp": monster_data.get("max_hp"),
            "movementSpeed": monster_data.get("movement_speed"),
            "name": monster_data.get("name"),
            "podName": monster_data.get("pod_name"),
            "position": monster_data.get("position"),
            "spawnTimestamp": monster_data.get("spawn_timestamp"),
            "turnsBetweenRegen": monster_data.get("turns_between_regen"),
            "type": monster_data.get("type"),
        }

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
            raise RuntimeError(f"Failed to create Monster resource: {e}") from e

    def update_monster_resource(self, name: str, namespace: str, monster_data: dict,
                                retries: int = 3):
        """
        Update an existing Monster custom resource.

        Args:
            name (str): The name of the Monster resource.
            namespace (str): The Kubernetes namespace where the resource is located.
            monster_data (dict): Dictionary containing the fields to update in the spec.
            retries (int): Number of retries for handling conflict errors.

        Raises:
            ValueError: If the resource does not have a 'spec' field.
            client.exceptions.ApiException: If there is an error with the API request.
        """
        if not self.resource_exists(name, namespace):
            current_app.logger.warning(f"Monster resource {name} does not exist in namespace \
                {namespace} - unable to update")
            return

        # Convert keys in monster_data to camelCase
        monster_data = {
            "accuracy": monster_data.get("accuracy"),
            "attackSpeed": monster_data.get("attack_speed"),
            "damageMax": monster_data.get("damage_max"),
            "damageMin": monster_data.get("damage_min"),
            "deathTimestamp": monster_data.get("death_timestamp"),
            "defense": monster_data.get("defense"),
            "depth": monster_data.get("depth"),
            "hp": monster_data.get("hp"),
            "id": monster_data.get("id"),
            "isDead": monster_data.get("is_dead"),
            "maxHp": monster_data.get("max_hp"),
            "movementSpeed": monster_data.get("movement_speed"),
            "name": monster_data.get("name"),
            "podName": monster_data.get("pod_name"),
            "position": monster_data.get("position"),
            "spawnTimestamp": monster_data.get("spawn_timestamp"),
            "turnsBetweenRegen": monster_data.get("turns_between_regen"),
            "type": monster_data.get("type"),
        }

        for attempt in range(retries):
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
                return
            except ApiException as e:
                if e.status == 409 and attempt < retries - 1:
                    current_app.logger.warning(f"Conflict error while updating Monster resource \
                        {name}, retrying...")
                    time.sleep(1)  # Wait before retrying
                else:
                    current_app.logger.error(f"Failed to update Monster resource: {e}")
                    raise e
            except ValueError as e:
                current_app.logger.error(f"Invalid resource structure: {e}")
                raise e

    def delete_monster_resource(self, name: str, namespace: str):
        """
        Delete a specific Monster custom resource.

        Args:
            name (str): The name of the Monster resource to delete.
            namespace (str): The Kubernetes namespace where the resource is located.

        Raises:
            client.exceptions.ApiException: If there is an error with the API request.
        """
        if not self.resource_exists(name, namespace):
            current_app.logger.warning(f"Monster resource {name} does not exist in namespace \
                {namespace} - unable to delete")
            return

        try:
            self.api.delete_namespaced_custom_object(
                group="kaschaefer.com",
                version="v1",
                namespace=namespace,
                plural="monsters",
                name=name,
            )
            current_app.logger.info(
                f"Successfully deleted Monster resource: {name} in namespace {namespace}"
            )
        except client.exceptions.ApiException as e:
            current_app.logger.error(
                f"Failed to delete Monster resource {name} in namespace {namespace}: {e}"
            )
            raise e
        except Exception as e:
            current_app.logger.error(
                f"Unexpected error while deleting Monster resource {name}: {e}"
            )
            raise e

    def list_monsters_in_namespace(self, namespace: str):
        """
        List all Monster custom resources in a specific namespace.

        Args:
            namespace (str): The Kubernetes namespace to query.

        Returns:
            list: A list of Monster resources in the specified namespace.

        Raises:
            client.exceptions.ApiException: If there is an error with the API request.
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
        Retrieve a specific Monster custom resource.

        Args:
            name (str): The name of the Monster resource.
            namespace (str): The Kubernetes namespace where the resource is located.

        Returns:
            dict or None: The Monster resource data or None if not found.

        Raises:
            client.exceptions.ApiException: If there is an error with the API request.
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
        Delete all Monster custom resources in a specific namespace.

        Args:
            namespace (str): The Kubernetes namespace containing the resources to delete.

        Raises:
            client.exceptions.ApiException: If there is an error with the API request.
        """
        try:
            monsters = self.list_monsters_in_namespace(namespace)

            if not monsters:
                current_app.logger.info(f"No Monster resources found in namespace {namespace}")
                return

            for monster in monsters:
                name = monster.get("metadata", {}).get("name")
                if name and self.resource_exists(name, namespace):
                    self.delete_monster_resource(name, namespace)
                else:
                    current_app.logger.warning(f"\
                        Monster resource missing 'name' field or does not exist: {monster}")

        except client.exceptions.ApiException as e:
            current_app.logger.error(f"\
                Failed to delete Monster resources in namespace {namespace}: {e}")
            raise e
        except Exception as e:
            current_app.logger.error(f"\
                Unexpected error while deleting Monster resources in namespace {namespace}: {e}")
            raise e
