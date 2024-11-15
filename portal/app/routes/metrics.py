from flask import Blueprint
from prometheus_client import generate_latest, CONTENT_TYPE_LATEST

# Register metrics route under the Blueprint
bp = Blueprint('metrics', __name__)

@bp.route('/', methods=['GET'])
def metrics():
    """
    Endpoint to serve Prometheus metrics.
    Returns metrics in a format suitable for Prometheus scraping.
    """
    return generate_latest(), 200, {'Content-Type': CONTENT_TYPE_LATEST}
