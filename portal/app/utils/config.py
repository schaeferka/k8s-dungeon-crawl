import os
from dotenv import load_dotenv

# Load environment variables from a .env file if it exists
load_dotenv()

class Config:
    """Utility class for accessing app configuration."""

    @staticmethod
    def get(key: str, default=None):
        """
        Retrieve a configuration value.

        Args:
            key (str): The name of the configuration key.
            default: The default value to return if the key is not found.

        Returns:
            str: The configuration value or the default if not found.
        """
        return os.getenv(key, default)

    @staticmethod
    def is_production() -> bool:
        """Check if the app is running in production mode."""
        return Config.get("FLASK_CONFIG", "default") == "production"

    @staticmethod
    def is_development() -> bool:
        """Check if the app is running in development mode."""
        return Config.get("FLASK_CONFIG", "default") == "development"

    @staticmethod
    def flask_config() -> str:
        """
        Get the current Flask configuration mode.

        Returns:
            str: The current configuration mode (e.g., 'development', 'production').
        """
        return Config.get("FLASK_CONFIG", "default")

    @staticmethod
    def portal_namespace() -> str:
        """
        Get the namespace where the portal is running.

        Returns:
            str: The portal namespace.
        """
        return Config.get("PORTAL_NAMESPACE", "portal")

    @staticmethod
    def monster_crd_info() -> dict:
        """
        Get information about the Monster CRD.

        Returns:
            dict: A dictionary containing CRD group, version, and plural name.
        """
        return {
            "group": Config.get("MONSTER_CRD_GROUP", "kaschaefer.com"),
            "version": Config.get("MONSTER_CRD_VERSION", "v1"),
            "plural": Config.get("MONSTER_CRD_PLURAL", "monsters"),
        }
