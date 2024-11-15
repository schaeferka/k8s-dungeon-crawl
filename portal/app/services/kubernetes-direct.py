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
        self.api = client.CoreV1Api()
        self.apps_api = client.AppsV1Api()

    def create_monster_deployment(self, name: str, namespace: str, monster_data: dict):
        """
        Create a deployment for a monster.
        :param name: Name of the deployment.
        :param namespace: Kubernetes namespace to create the deployment in.
        :param monster_data: Dictionary containing monster-specific data (e.g., type, health).
        """
        deployment_manifest = {
            "apiVersion": "apps/v1",
            "kind": "Deployment",
            "metadata": {"name": name, "namespace": namespace},
            "spec": {
                "replicas": 1,
                "selector": {"matchLabels": {"app": name}},
                "template": {
                    "metadata": {"labels": {"app": name}},
                    "spec": {
                        "containers": [
                            {
                                "name": "nginx",
                                "image": "nginx:latest",
                                "ports": [{"containerPort": 80}],
                                "volumeMounts": [
                                    {
                                        "name": "monster-config",
                                        "mountPath": "/usr/share/nginx/html/index.html",
                                        "subPath": "index.html",
                                    }
                                ],
                            }
                        ],
                        "volumes": [
                            {
                                "name": "monster-config",
                                "configMap": {"name": f"{name}-config"},
                            }
                        ],
                    },
                },
            },
        }

        try:
            self.apps_api.create_namespaced_deployment(namespace=namespace, body=deployment_manifest)
            logger.info(f"Created deployment for monster: {name}")
        except client.exceptions.ApiException as e:
            logger.error(f"Failed to create deployment: {e}")
            raise e

    def create_monster_configmap(self, name: str, namespace: str, monster_data: dict):
        """
        Create a ConfigMap for the monster to customize its index.html.
        :param name: Name of the ConfigMap.
        :param namespace: Kubernetes namespace to create the ConfigMap in.
        :param monster_data: Dictionary containing monster-specific data (e.g., type, health).
        """
        monster_html = f"""
        <html>
            <head><title>{monster_data.get('name', 'Monster')}</title></head>
            <body>
                <h1>{monster_data.get('name', 'Monster')}</h1>
                <p>Type: {monster_data.get('type', 'Unknown')}</p>
                <p>Health: {monster_data.get('health', 'Unknown')}</p>
            </body>
        </html>
        """

        configmap_manifest = {
            "apiVersion": "v1",
            "kind": "ConfigMap",
            "metadata": {"name": f"{name}-config", "namespace": namespace},
            "data": {"index.html": monster_html},
        }

        try:
            self.api.create_namespaced_config_map(namespace=namespace, body=configmap_manifest)
            logger.info(f"Created ConfigMap for monster: {name}")
        except client.exceptions.ApiException as e:
            logger.error(f"Failed to create ConfigMap: {e}")
            raise e

    def delete_monster_resources(self, name: str, namespace: str):
        """
        Delete the deployment and ConfigMap for a monster.
        :param name: Name of the monster (used for the deployment and ConfigMap names).
        :param namespace: Kubernetes namespace to delete the resources from.
        """
        try:
            self.apps_api.delete_namespaced_deployment(name=name, namespace=namespace)
            logger.info(f"Deleted deployment for monster: {name}")
        except client.exceptions.ApiException as e:
            logger.warning(f"Failed to delete deployment: {e}")

        try:
            self.api.delete_namespaced_config_map(name=f"{name}-config", namespace=namespace)
            logger.info(f"Deleted ConfigMap for monster: {name}")
        except client.exceptions.ApiException as e:
            logger.warning(f"Failed to delete ConfigMap: {e}")

    def list_monsters_in_namespace(self, namespace: str):
        """
        List all deployments in the namespace that represent monsters.
        :param namespace: Kubernetes namespace to query.
        :return: List of monster deployment names.
        """
        try:
            deployments = self.apps_api.list_namespaced_deployment(namespace=namespace)
            monster_deployments = [deploy.metadata.name for deploy in deployments.items if "app" in deploy.metadata.labels]
            return monster_deployments
        except client.exceptions.ApiException as e:
            logger.error(f"Failed to list monster deployments: {e}")
            return []
