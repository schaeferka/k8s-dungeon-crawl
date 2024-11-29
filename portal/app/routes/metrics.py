"""
This module defines the `metrics` blueprint for serving Prometheus metrics.

The blueprint exposes a single route that serves the application metrics in a format
suitable for Prometheus scraping. This allows Prometheus to collect and monitor the
metrics exposed by the application.

Endpoints:
- /: Serves the Prometheus metrics.

Returns:
    None: This module does not return values directly but defines routes for the Flask application.
"""
from flask import Blueprint
from prometheus_client import generate_latest, CONTENT_TYPE_LATEST

# Register metrics route under the Blueprint
bp = Blueprint('metrics', __name__)

@bp.route('/', methods=['GET'])
def metrics():
    """
    Endpoint to serve Prometheus metrics.

    This route is used to expose the application's metrics in a format that is compatible
    with Prometheus scraping. When Prometheus scrapes this endpoint, it will receive the
    latest metrics in the format it expects, allowing it to track and monitor the application.

    The response includes:
        - The generated metrics as the body of the response.
        - A 200 HTTP status indicating success.
        - The `Content-Type` header set to `text/plain; version=0.0.4`, which is the appropriate
          content type for Prometheus metrics.

    Args:
        None: This is a GET endpoint with no parameters.

    Returns:
        tuple: A tuple containing the metrics, HTTP status code, and headers.
            - Metrics (str): The Prometheus-formatted metrics data.
            - Status code (int): 200 indicating the request was successful.
            - Headers (dict): A dictionary with the `Content-Type` set to `CONTENT_TYPE_LATEST`.
    """
    # Generate the latest Prometheus metrics
    return generate_latest(), 200, {'Content-Type': CONTENT_TYPE_LATEST}
