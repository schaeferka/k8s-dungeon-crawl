"""_summary_

Returns:
    _type_: _description_
"""
import subprocess
from flask import Blueprint, jsonify, request, current_app, render_template
try:
    from monsters import active_monsters
except ImportError:
    active_monsters = {}

bp = Blueprint('monsties', __name__)

monsties = []
new_monsties = []

@bp.route("/", methods=["GET"])
def monsties_page():
    """
    Endpoint to render the monsties page.
    """
    return render_template("monsties.html")

@bp.route('/add', methods=['POST'], strict_slashes=False)
def add_monster():
    """
    Adds a new monster to the new_monsties list.
    Expects a JSON payload with a 'pod-name' key.
    """
    data = request.json
    pod_name = data.get('pod-name', 'Unknown')  # Default to 'Unknown' if pod-name is missing
    new_monsties.append(pod_name)
    current_app.logger.info(f"Added {pod_name} to new monsties")
    current_app.logger.info(f"New monsties: {new_monsties}")
    return jsonify(monsties)

@bp.route('/add/<pod_name>', methods=['GET'])
def add_monster_by_name(pod_name):
    """
    Adds a new monster to the new_monsties list using a URL parameter.
    """
    new_monsties.append(pod_name)
    current_app.logger.info(f"Added {pod_name} to new monsties")
    current_app.logger.info(f"New monsties: {new_monsties}")
    return jsonify(monsties)

@bp.route('/new', methods=['GET'])
def get_new_monsties():
    """
    Returns the list of new monsties and moves them to the monsties list.
    """
    response = {"pod-names": new_monsties.copy()}
    #current_app.logger.info(f"Responding to request for list of new monsties with: {response}")
    monsties.extend(new_monsties)
    new_monsties.clear()
    return jsonify(response)

@bp.route('/list', methods=['GET'], strict_slashes=False)
def list_monsters():
    """
    Returns the list of all monsties.
    """
    return jsonify(monsties)

@bp.route('/reset', methods=['POST'], strict_slashes=False)
def reset_current_game_monsties():
    """
    Resets the current game state for monsties.
    """
    monsties.clear()
    new_monsties.clear()
    current_app.logger.info("Monstie data has been reset for a new game.")
    return jsonify({"status": "success"}), 200

@bp.route('/data', methods=['GET'])
def get_monsties():
    """Fetch all monsters of type 'monstie' from the active_monsters list."""
    try:
        monsties_list = [monster for monster in active_monsters.values() if monster.get('type') == 'monstie']
        return jsonify(monsties_list)
    except KeyError as e:
        return jsonify({'error': str(e)}), 500

@bp.route('/deployments-pods', methods=['GET'])
def get_monsties_deployments_pods():
    """Fetch deployments and pods in the monsties namespace."""
    try:
        deployments_output = subprocess.run([
            'kubectl', 'get', 'deployments', '-n', 'monsties', '-o', 'json'
        ], capture_output=True, text=True, check=True)
        pods_output = subprocess.run([
            'kubectl', 'get', 'pods', '-n', 'monsties', '-o', 'json'
        ], capture_output=True, text=True, check=True)
        return jsonify({'deployments': deployments_output.stdout, 'pods': pods_output.stdout})
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500

@bp.route('/admin-kill/<pod_name>', methods=['DELETE'])
def admin_kill_pod(pod_name):
    """Delete a pod in the monsties namespace."""
    try:
        subprocess.run(['kubectl', 'delete', 'pod', pod_name, '-n', 'monsties'], check=True)
        return jsonify({'message': f'Pod {pod_name} deleted successfully'}), 200
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500

@bp.route('/create-deployment', methods=['POST'])
def create_monstie_deployment():
    """Create a new monstie deployment with two replicas."""
    try:
        subprocess.run([
            'kubectl', 'create', 'deployment', 'monstie-deployment', '--image=nginx', '--replicas=2', '-n', 'monsties'
        ], check=True)
        return jsonify({'message': 'Monstie deployment created successfully'}), 201
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500
