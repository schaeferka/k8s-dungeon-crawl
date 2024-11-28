"""
This module defines the `items` blueprint for managing item-related metrics and data.

It provides routes for receiving item metrics data, updating both in-memory storage 
and Prometheus metrics, and retrieving the current item data. The module is designed to
track item metrics, such as weapon damage and armor defense, and store them for the application.

Returns:
    None: This module does not return values directly but defines routes for Flask application 
    and Prometheus metrics.
"""
from flask import Blueprint, request, jsonify, current_app
from app.models.items import EquippedItems
from prometheus_client import Gauge

# Create a Blueprint named 'items' for item-related routes
bp = Blueprint('items', __name__)

# Item Metrics (Prometheus Gauges for tracking item attributes)
item_weapon_damage_min = Gauge('brogue_weapon_damage_min', 'Minimum damage of equipped weapon')
item_weapon_damage_max = Gauge('brogue_weapon_damage_max', 'Maximum damage of equipped weapon')
item_armor_defense = Gauge('brogue_armor_defense', 'Defense of equipped armor')

# In-memory storage for items
items_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_items():
    """
    Receives item metrics data and updates in-memory storage as well as Prometheus metrics.

    This route handles POST requests that contain item-related metrics data (e.g., weapon damage,
    armor defense). It validates the incoming data using the `EquippedItems` model, updates both the 
    in-memory `items_data` storage, and updates the corresponding Prometheus metrics.

    Args:
        None: This is a POST endpoint that expects a JSON payload containing item metrics data.

    Returns:
        Response: A JSON response indicating the success or failure of the request, along with the 
        received data.
    """
    # Extract JSON data from the incoming request
    data = request.json

    # Check if the request contains data
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate and parse the incoming JSON using the EquippedItems model
        item_metrics = EquippedItems(**data)

        # Update the in-memory items data
        items_data.update({
            "weapon": item_metrics.weapon.model_dump(),
            "armor": item_metrics.armor.model_dump(),
            "ringLeft": item_metrics.left_ring.model_dump(),
            "ringRight": item_metrics.right_ring.model_dump()
        })

        # Update Prometheus metrics for weapon damage and armor defense
        if item_metrics.weapon and item_metrics.weapon.damage:
            item_weapon_damage_min.set(item_metrics.weapon.damage.min)
            item_weapon_damage_max.set(item_metrics.weapon.damage.max)

        if item_metrics.armor:
            item_armor_defense.set(item_metrics.armor.armor)

        # Return a success response along with the received item metrics data
        return jsonify({"status": "success", "received": item_metrics.model_dump()}), 200

    except ValueError as e:
        # Log the error and return a failure response if a ValueError occurs
        current_app.logger.error(f"ValueError processing item data: {e}")
        return jsonify({"error": str(e)}), 400
    except TypeError as e:
        # Log the error and return a failure response if a TypeError occurs
        current_app.logger.error(f"TypeError processing item data: {e}")
        return jsonify({"error": str(e)}), 400
    except KeyError as e:
        # Log the error and return a failure response if a KeyError occurs
        current_app.logger.error(f"KeyError processing item data: {e}")
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_items():
    """
    Returns the current item data.

    This route handles GET requests to retrieve the current items data that has been
    stored in memory. If no data is available, an empty JSON object is returned.

    Args:
        None: This is a GET endpoint that returns the current in-memory item data.

    Returns:
        Response: A JSON response containing the current item data stored in memory.
    """
    return jsonify(items_data)
