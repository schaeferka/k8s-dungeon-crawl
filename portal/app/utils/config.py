"""
This module defines the `Config` class, a utility for accessing application configuration.

The configuration values are loaded from environment variables, typically set in a `.env` file.
The `Config` class provides methods for retrieving specific configuration values, checking the 
current environment (e.g., production or development), and accessing specific application settings 
such as the portal namespace and monster CRD information.

Returns:
    None: This module does not return any values.
"""
import os
from dotenv import load_dotenv

# Load environment variables from a .env file if it exists
load_dotenv()

class Config:
    """Utility class for accessing app configuration.

    This class provides static methods for retrieving configuration values, checking 
    the current environment, and fetching specific app settings. It interacts with 
    environment variables loaded via the `dotenv` package.

    Attributes:
        None
    """

    @staticmethod
    def get(key: str, default=None):
        """
        Retrieve a configuration value.

        This method looks up a configuration value from environment variables and returns
        it. If the key is not found, it returns the specified default value.

        Args:
            key (str): The name of the configuration key.
            default: The default value to return if the key is not found.

        Returns:
            str: The configuration value or the default if not found.
        """
        return os.getenv(key, default)

    @staticmethod
    def is_production() -> bool:
        """
        Check if the app is running in production mode.

        This method checks the `FLASK_CONFIG` environment variable to determine if 
        the application is configured for production. If the environment variable 
        is not set to "production", it returns `False`.

        Returns:
            bool: `True` if the app is in production mode, `False` otherwise.
        """
        return Config.get("FLASK_CONFIG", "default") == "production"

    @staticmethod
    def is_development() -> bool:
        """
        Check if the app is running in development mode.

        This method checks the `FLASK_CONFIG` environment variable to determine if 
        the application is configured for development. If the environment variable 
        is not set to "development", it returns `False`.

        Returns:
            bool: `True` if the app is in development mode, `False` otherwise.
        """
        return Config.get("FLASK_CONFIG", "default") == "development"

    @staticmethod
    def flask_config() -> str:
        """
        Get the current Flask configuration mode.

        This method retrieves the current configuration mode from the `FLASK_CONFIG`
        environment variable. It returns the mode (e.g., 'development', 'production'),
        or the default value ('default') if the variable is not set.

        Returns:
            str: The current configuration mode (e.g., 'development', 'production').
        """
        return Config.get("FLASK_CONFIG", "default")

    @staticmethod
    def portal_namespace() -> str:
        """
        Get the namespace where the portal is running.

        This method retrieves the namespace for the portal service from the environment 
        variable `PORTAL_NAMESPACE`. If the variable is not set, it defaults to `"portal"`.

        Returns:
            str: The portal namespace.
        """
        return Config.get("PORTAL_NAMESPACE", "portal")

    @staticmethod
    def monster_crd_info() -> dict:
        """
        Get information about the Monster CRD (Custom Resource Definition).

        This method returns a dictionary with information about the Monster CRD,
        including the group, version, and plural name. The values are fetched from 
        the environment variables (`MONSTER_CRD_GROUP`, `MONSTER_CRD_VERSION`, 
        and `MONSTER_CRD_PLURAL`). Defaults are provided if the environment variables
        are not set.

        Returns:
            dict: A dictionary containing CRD group, version, and plural name.
        """
        return {
            "group": Config.get("MONSTER_CRD_GROUP", "kaschaefer.com"),
            "version": Config.get("MONSTER_CRD_VERSION", "v1"),
            "plural": Config.get("MONSTER_CRD_PLURAL", "monsters"),
        }
