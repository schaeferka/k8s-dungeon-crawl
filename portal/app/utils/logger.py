import logging
import sys

def configure_logger(app, name: str = "portal"):
    """
    Configure and attach a logger to the Flask app instance.

    Args:
        app: The Flask app instance.
        name (str): The name of the logger (default is 'portal').
    """
    logger = logging.getLogger(name)

    # Avoid duplicate handlers if logger already configured
    if logger.hasHandlers():
        return logger

    # Set the log level
    logger.setLevel(logging.INFO)

    # Create a console handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(logging.INFO)

    # Define log format
    formatter = logging.Formatter(
        fmt="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S"
    )
    console_handler.setFormatter(formatter)

    # Add the handler to the logger
    logger.addHandler(console_handler)

    # Attach the logger to the Flask app
    app.logger = logger

    return logger
