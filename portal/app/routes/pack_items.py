"""
This module defines the API endpoints for managing pack items in the K8s Dungeon Crawl game.

It includes the following functionalities:

1. Receiving and updating pack item data via a POST request, validating the input using the Pack model.
2. Providing the current state of the pack items data.

Endpoints:
- /update: Receives pack item data via a POST request and updates the in-memory storage.
- /data: Returns the current pack items data as a JSON response.

The pack items data is stored in an in-memory dictionary and validated using the Pack model.
"""
from flask import Blueprint, request, jsonify, current_app
from app.models.items import Pack

bp = Blueprint('pack', __name__)

pack_items = {chr(i): None for i in range(ord('a'), ord('z') + 1)}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_pack_items():
    """
    Receives pack item metrics data and updates in-memory storage as well as Prometheus metrics.

    This route validates the incoming data using the `Pack` model and updates the 
    in-memory `pack_items` storage.

    Args:
        None: This is a POST endpoint that expects a JSON payload containing pack item data.

    Returns:
        Response: A JSON response indicating the success or failure of the request, along with the 
        received pack item data.
    """
    data = request.json

    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate and parse the incoming JSON using the Pack model
        items = Pack(**data)

        for new_item in items.pack:
            pack_items[new_item.inventory_letter] = new_item

        return jsonify({"status": "success", "received": items.dict()}), 200

    except (ValueError, TypeError, KeyError) as e:
        current_app.logger.error(f"Error processing pack item data: {e}")
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_pack_items():
    """
    Returns the current pack items data.

    This route handles GET requests to retrieve the current in-memory pack items.
    If no pack items data is available, it returns a 404 error with an appropriate message.

    Args:
        None: This is a GET endpoint that returns the current pack items data.

    Returns:
        Response: A JSON response containing the current pack items data, or an error message if 
        the data is not available.
    """
    if not pack_items:
        return jsonify({"error": "Pack items data not available"}), 404

    pack_items_data = {
        "pack": {letter: item.dict() for letter, item in pack_items.items() if item}
    }

    return jsonify(pack_items_data)
