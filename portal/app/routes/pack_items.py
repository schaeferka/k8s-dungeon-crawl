"""
This module defines the `packitems` blueprint for managing pack items and exposing 
related metrics.

It provides routes for receiving pack item data, updating in-memory storage, 
and Prometheus metrics. The `Pack` model is used to validate and store the structured 
data for pack items.

Returns:
    None: This module does not return values directly but provides Flask routes and 
    Prometheus metrics.
"""
from flask import Blueprint, request, jsonify, current_app
from app.models.items import Pack

bp = Blueprint('packitems', __name__)

pack_items = {}

def handle_pack_items(data):
    """
    Helper function to handle the pack items and update the in-memory storage.

    This function validates and parses the incoming pack item data using the 
    `Pack` model. It updates the in-memory storage for the pack items.

    Args:
        data (list): The list of pack items, typically extracted from the request body.

    Returns:
        None: The function updates the in-memory `pack_items` dictionary.
    """
    # Validate and parse the incoming data using the Pack model
    pack_items_data = [Pack(**item) for item in data]

    # Store the Pack objects themselves (not just JSON) in in-memory storage
    pack_items["pack"] = pack_items_data

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_pack_items():
    """
    Receives pack item data and updates in-memory storage.

    This route handles POST requests that contain pack item data. It validates the incoming 
    data using the `Pack` model, updates both the in-memory `pack_items` storage.

    Args:
        None: This is a POST endpoint that expects a JSON payload containing pack item data.

    Returns:
        Response: A JSON response indicating the success or failure of the request, along with 
        the received data.
    """
    data = request.json

    # Check if the request contains data
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Ensure that the "pack" field is present in the incoming data
        if "pack" not in data:
            return jsonify({"error": "'pack' field is missing in the request data"}), 400

        # Process the pack items data
        handle_pack_items(data["pack"])

        # Return a success response along with the received data
        return jsonify({"status": "success", "received": data}), 200

    except (ValueError, TypeError, KeyError) as e:
        # Log the error and return a failure response if an error occurs
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
    # Check if the pack items data exists in memory
    if not pack_items:
        return jsonify({"error": "Pack items data not available"}), 404

    pack_items_data = [item.model_dump() for item in pack_items.get("pack", [])]
    
    return jsonify({"pack": pack_items_data})
