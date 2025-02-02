"""_summary_

Returns:
    _type_: _description_
"""
from flask import Blueprint, jsonify, request, current_app

bp = Blueprint('monsties', __name__)

monsties = []
new_monsties = []

@bp.route('/add', methods=['POST'], strict_slashes=False)
def add_monster():
    """
    Adds a new monster to the new_monsties list.

    Expects a JSON payload with a 'pod-name' key.
    """
    data = request.json
    new_monsties.append(data['pod-name'])
    current_app.logger.info(f"Added {data['pod-name']} to new monsties")
    current_app.logger.info(f"New monsties: {new_monsties}")
    return jsonify(monsties)

@bp.route('/add/<pod_name>', methods=['GET'])
def add_monster_by_name(pod_name):
    """
    Adds a new monster to the new_monsties list using a URL parameter.

    @param pod_name: The name of the pod to add.
    """
    new_monsties.append(pod_name)
    current_app.logger.info(f"Added {pod_name} to new monsties")
    current_app.logger.info(f"New monsties: {new_monsties}")
    return jsonify(monsties)

@bp.route('/new', methods=['GET'])
def get_new_monsties():
    """
    Returns the list of new monsties and moves them to the monsties list.

    The response contains a JSON object with a 'pod-names' key.
    """
    response = {"pod-names": new_monsties.copy()}
    current_app.logger.info(f"Responding to request for list of new monsties with: {response}")
    monsties.extend(new_monsties)
    new_monsties.clear()
    return jsonify(response)

@bp.route('/list', methods=['GET'], strict_slashes=False)
def list_monsters():
    """
    Returns the list of all monsties.

    The response contains a JSON array of monsties.
    """
    return jsonify(monsties)

@bp.route('/reset', methods=['POST'], strict_slashes=False)
def reset_current_game_monsties():
    """
    Resets the current game state for monsties.

    Clears the in-memory data for monsties.

    Returns:
        Response: A JSON response indicating the status of the reset.
    """
    monsties.clear()
    new_monsties.clear()

    current_app.logger.info("Monstie data has been reset for a new game.")
    return jsonify({"status": "success"}), 200
