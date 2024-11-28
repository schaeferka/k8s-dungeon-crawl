"""
This module defines the `items` blueprint for managing equipped items and exposing 
related metrics.

It provides routes for receiving equipped item metrics data, updating in-memory storage, 
and Prometheus metrics. The `EquippedItems` model is used to validate and store the structured 
data for equipped items, including weapons, armor, and rings.

Prometheus metrics are updated based on the attributes of the equipped items, such as weapon damage 
and armor defense.

Returns:
    None: This module does not return values directly but provides Flask routes and 
    Prometheus metrics.
"""

from flask import Blueprint, request, jsonify, current_app
from app.models.items import EquippedItems
from prometheus_client import Gauge

bp = Blueprint('items', __name__)

# Item Metrics (Prometheus Gauges for tracking in-game data)
item_weapon_damage_min = Gauge('brogue_weapon_damage_min', 'Minimum damage of equipped weapon')
item_weapon_damage_max = Gauge('brogue_weapon_damage_max', 'Maximum damage of equipped weapon')
item_armor_defense = Gauge('brogue_armor_defense', 'Defense of equipped armor')

# In-memory storage for equipped items
equipped_items = {}

def handle_equipped_items(data):
    """
    Helper function to handle equipped items and update the in-memory storage.
    
    This function validates and parses the incoming equipped item data using the 
    `EquippedItems` model. It updates the in-memory storage for the equipped items 
    and updates corresponding Prometheus metrics for weapon damage and armor defense.
    
    Args:
        data (dict): The data for equipped items, typically extracted from the request body.

    Returns:
        None: The function updates the in-memory `equipped_items` dictionary and Prometheus metrics.
    """
    equipped_items_data = EquippedItems(**data)

    equipped_items["weapon"] = equipped_items_data.weapon
    equipped_items["armor"] = equipped_items_data.armor
    equipped_items["ringLeft"] = equipped_items_data.left_ring
    equipped_items["ringRight"] = equipped_items_data.right_ring

    # Update Prometheus metrics for weapon damage and armor defense
    if equipped_items_data.weapon and equipped_items_data.weapon.damage:
        item_weapon_damage_min.set(equipped_items_data.weapon.damage.min)
        item_weapon_damage_max.set(equipped_items_data.weapon.damage.max)

    if equipped_items_data.armor:
        item_armor_defense.set(equipped_items_data.armor.armor)

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_equipped_items():
    """
    Receives equipped item metrics data and updates in-memory storage as well as Prometheus metrics.

    This route handles POST requests that contain equipped item metrics data (e.g., weapon damage,
    armor defense). It validates the incoming data using the `EquippedItems` model, updates both the 
    in-memory `equipped_items` storage, and updates the corresponding Prometheus metrics.

    Args:
        None: This is a POST endpoint that expects a JSON payload containing equipped item metrics data.

    Returns:
        Response: A JSON response indicating the success or failure of the request, along with the 
        received equipped item data.
    """
    # Extract JSON data from the incoming request
    data = request.json

    # Check if the request contains data
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate and parse the incoming JSON using the EquippedItems model
        items = EquippedItems(**data)

        # Update the in-memory items data
        equipped_items.update({
            "weapon": items.weapon,
            "armor": items.armor,
            "ringLeft": items.left_ring,
            "ringRight": items.right_ring
        })

        # Update Prometheus metrics for weapon damage and armor defense
        if items.weapon and items.weapon.damage:
            item_weapon_damage_min.set(items.weapon.damage.min)
            item_weapon_damage_max.set(items.weapon.damage.max)

        if items.armor:
            item_armor_defense.set(items.armor.armor)

        # Return a success response along with the received item metrics data
        return jsonify({"status": "success", "received": items.model_dump()}), 200

    except (ValueError, TypeError, KeyError) as e:
        # Log the error and return a failure response if an error occurs
        current_app.logger.error(f"Error processing equipped item data: {e}")
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_equipped_items():
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
    # Check if the equipped items data exists in memory
    if not equipped_items:
        return jsonify({"error": "Equipped items data not available"}), 404

    # Return the equipped items data as a JSON response, serializing the objects using model_dump()
    equipped_items_data = {key: item.model_dump() for key, item in equipped_items.items()}

    return jsonify(equipped_items_data)