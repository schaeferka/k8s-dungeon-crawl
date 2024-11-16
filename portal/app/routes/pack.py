from flask import Blueprint, request, jsonify, current_app
from app.models.pack import Pack

bp = Blueprint('pack', __name__)

pack_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_pack():
    """
    Receives pack data and updates the in-memory storage.
    """
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    if "pack" not in data:
        return jsonify({"error": "Missing 'pack' field in data"}), 400

    # Optional: Validate structure of each item in the pack
    for item in data.get("pack", []):
        if not all(k in item for k in ["category", "name", "quantity", "inventoryLetter"]):
            return jsonify({"error": "Item missing required fields"}), 400

    # Store or update the pack data
    pack_data.update(data)

    # current_app.logger.info("Received and updated pack data: %s", data)
    
    return jsonify({"status": "success", "received": data}), 200

@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_pack():
    """
    Returns the current pack data.
    """
    return jsonify(pack_data)
