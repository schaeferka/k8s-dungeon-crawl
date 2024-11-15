from flask import Blueprint, request, jsonify, current_app
from app.models.items import Items
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST

bp = Blueprint('items', __name__)

# Item Metrics
item_weapon_damage_min = Gauge('brogue_weapon_damage_min', 'Minimum damage of equipped weapon')
item_weapon_damage_max = Gauge('brogue_weapon_damage_max', 'Maximum damage of equipped weapon')
item_armor_defense = Gauge('brogue_armor_defense', 'Defense of equipped armor')

# In-memory storage for items
items_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_items():
    """
    Receives items data and updates the in-memory storage, 
    as well as Prometheus metrics.
    """
    data = request.json
    current_app.logger.info("Received item metrics data: %s", data)
    
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate required fields
        required_fields = ['name', 'category', 'damage_min', 'damage_max', 'defense']
        for field in required_fields:
            if field not in data:
                return jsonify({"error": f"Missing required field: {field}"}), 400
        
        # Process and update items data
        item = Items(**data)
        items_data.update({item.name: data})  # Use item name as the key

        # Update Prometheus metrics for weapon damage and armor defense
        if 'damage_min' in data:
            item_weapon_damage_min.set(data['damage_min'])
        if 'damage_max' in data:
            item_weapon_damage_max.set(data['damage_max'])
        if 'defense' in data:
            item_armor_defense.set(data['defense'])

        return jsonify({"status": "success", "received": item.dict()}), 200
    except Exception as e:
        current_app.logger.error(f"Error processing item data: {e}")
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_items():
    """
    Returns the current items data.
    """
    return jsonify(items_data)
