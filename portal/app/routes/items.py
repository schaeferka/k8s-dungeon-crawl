from flask import Blueprint, request, jsonify, current_app
from app.models.items import ItemMetrics, Item, Damage
from prometheus_client import Gauge

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
        # Validate and parse incoming data using Pydantic model
        item_metrics = ItemMetrics(**data)
        
        # Update in-memory storage
        items_data.update({
            "weapon": item_metrics.weapon.dict(),
            "armor": item_metrics.armor.dict(),
            "ringLeft": item_metrics.ringLeft.dict(),
            "ringRight": item_metrics.ringRight.dict()
        })

        # Update Prometheus metrics for weapon damage and armor defense
        if item_metrics.weapon and item_metrics.weapon.damage:
            item_weapon_damage_min.set(item_metrics.weapon.damage.min)
            item_weapon_damage_max.set(item_metrics.weapon.damage.max)
        
        if item_metrics.armor:
            item_armor_defense.set(item_metrics.armor.armor)

        return jsonify({"status": "success", "received": item_metrics.dict()}), 200
    except Exception as e:
        current_app.logger.error(f"Error processing item data: {e}")
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_items():
    """
    Returns the current items data.
    """
    return jsonify(items_data)
