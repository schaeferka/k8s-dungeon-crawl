"""
This module defines the `pack` blueprint for managing pack items and exposing 
related metrics.

It provides routes for receiving pack item metrics data and updating in-memory storage, 
The `Pack` model is used to validate and store the structured data for pack items, 
including weapons, armor, and rings.

Returns:
    None: This module does not return values directly but provides Flask routes and 
    Prometheus metrics.
"""

from flask import Blueprint, request, jsonify, current_app
from app.models.items import Pack

bp = Blueprint('pack', __name__)

# In-memory storage for pack items
pack_items = []

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
    # Extract JSON data from the incoming request
    data = request.json

    # Check if the request contains data
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate and parse the incoming JSON using the Pack model
        items = Pack(**data)

        # Helper function to check if an item already exists in the pack
        def item_exists(new_item, existing_pack):
            # Define your unique identifier (e.g., inventory_letter, category, kind)
            for item in existing_pack:
                # Access attributes using dot notation
                if item.inventory_letter == new_item.inventory_letter:
                    return True  # Item with the same inventory letter exists
            return False

        # If pack_items is empty, initialize it, otherwise append only unique items
        if not pack_items:
            pack_items.append({
                "pack": items.pack
            })
        else:
            # Only add new items that do not already exist in the pack
            for new_item in items.pack:
                if not item_exists(new_item, pack_items[0]["pack"]):
                    pack_items[0]["pack"].append(new_item)

        # Return a success response along with the received item metrics data
        return jsonify({"status": "success", "received": items.model_dump()}), 200

    except (ValueError, TypeError, KeyError) as e:
        # Log the error and return a failure response if an error occurs
        current_app.logger.error(f"Error processing equipped item data: {e}")
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_pack_items():
    """
    Returns the current equipped items data.

    This route handles GET requests to retrieve the current in-memory equipped items.
    If no equipped items data is available, it returns a 404 error with an appropriate message.

    Args:
        None: This is a GET endpoint that returns the current equipped items data.

    Returns:
        Response: A JSON response containing the current equipped items data, or an error message if 
        the data is not available.
    """
    # Check if the equipped items data exists
    if not pack_items:
        return jsonify({"error": "Equipped items data not available"}), 404

    # Since you want the response to be a single JSON object, not a list of dictionaries,
    # we will return the pack data directly (without wrapping it in a list).
    pack_items_data = {
        "pack": [item.model_dump() for item in pack_items[0]["pack"]]  # Access the first pack in the list and serialize
    }

    # Return the equipped items data as a JSON response
    return jsonify(pack_items_data)