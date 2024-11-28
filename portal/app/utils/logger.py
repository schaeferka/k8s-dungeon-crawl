"""
This module configures and attaches a logger to a Flask app instance.

The `configure_logger` function initializes a logger, sets up its log level, formats the log output, 
and attaches the logger to the Flask app instance. It ensures that the logger is not duplicated 
by checking if a logger with the same name already exists.

Returns:
    logging.Logger: The configured logger instance.
"""
import logging
import sys

def configure_logger(app, name: str = "portal"):
    """
    Configure and attach a logger to the Flask app instance.

    This function creates a logger for the given Flask app, sets its log level to `INFO`, and adds
    a console handler to output logs to the standard output (`sys.stdout`). The log format includes
    the timestamp, logger name, log level, and the log message.

    If the logger has already been configured (i.e., if it already has handlers), the function 
    will return the existing logger without reconfiguring it.

    Args:
        app: The Flask app instance that the logger will be attached to.
        name (str): The name of the logger (default is 'portal'). This is used to identify the 
        logger.

    Returns:
        logging.Logger: The configured logger instance attached to the Flask app.
    """
    # Retrieve the logger by name (or create it if it doesn't exist)
    logger = logging.getLogger(name)

    # Avoid duplicate handlers if the logger has already been configured
    if logger.hasHandlers():
        return logger

    # Set the logging level to INFO
    logger.setLevel(logging.INFO)

    # Create a StreamHandler to output logs to stdout (console)
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(logging.INFO)  # Set the handler's log level

    # Define a log format with timestamp, logger name, log level, and message
    formatter = logging.Formatter(
        fmt="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S"
    )
    # Attach the formatter to the handler
    console_handler.setFormatter(formatter)

    # Add the console handler to the logger
    logger.addHandler(console_handler)

    # Attach the configured logger to the Flask app instance
    app.logger = logger

    return logger
