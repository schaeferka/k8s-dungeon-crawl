"""_summary_

Returns:
    _type_: _description_
"""
import subprocess
import uuid
import json
from flask import Blueprint, jsonify, request, current_app, render_template
import requests
from app.routes.monsters import get_monsters


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
    # Default to 'Unknown' if pod-name is missing
    pod_name = data.get('pod-name', 'Unknown')
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
    # current_app.logger.info(f"Responding to request for list of new monsties with: {response}")
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
    """Fetch all monsters of type 'monstie' using the get_monsters function from monsters.py."""
    try:
        all_monsters = get_monsters().json
        monsties_list = [monster for monster in all_monsters if monster.get('type') == 'monstie']
        return jsonify(monsties_list)
    except (requests.exceptions.RequestException, ValueError) as e:
        current_app.logger.error(f"Error fetching monsties: {e}")
        return jsonify({'error': str(e)}), 500


@bp.route('/deployments-pods', methods=['GET'])
def get_monsties_deployments_pods():
    """Fetch deployments and pods in the monsties namespace, ensuring correct mapping."""
    try:
        deployments_output = subprocess.run([
            'kubectl', 'get', 'deployments', '-n', 'monsties', '-o', 'json'
        ], capture_output=True, text=True, check=True)

        pods_output = subprocess.run([
            'kubectl', 'get', 'pods', '-n', 'monsties', '-o', 'json'
        ], capture_output=True, text=True, check=True)

        deployments_data = json.loads(deployments_output.stdout)
        pods_data = json.loads(pods_output.stdout)

        # Create a mapping from ReplicaSets to Deployments
        deployment_map = {}
        for deployment in deployments_data.get("items", []):
            deployment_name = deployment["metadata"]["name"]
            deployment_map[deployment_name] = {
                "replicas": deployment["spec"].get("replicas", 1),
                "pods": []
            }

        # Match pods to deployments via ReplicaSets
        for pod in pods_data.get("items", []):
            pod_name = pod["metadata"]["name"]
            owner_references = pod["metadata"].get("ownerReferences", [])

            # Find the ReplicaSet owner
            replica_set_name = next(
                (owner["name"] for owner in owner_references if owner["kind"] == "ReplicaSet"),
                None
            )

            # Map ReplicaSet to Deployment
            for dep_name in deployment_map:
                if replica_set_name and replica_set_name.startswith(dep_name):
                    deployment_map[dep_name]["pods"].append({
                        "name": pod_name,
                        "status": pod["status"]["phase"]
                    })

        return jsonify(deployment_map)
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500

@bp.route('/create-deployment', methods=['POST'])
def create_monstie_deployment():
    """Create a new monstie deployment with two replicas."""
    deployment_name = f"monstie-deployment-{uuid.uuid4().hex[:6]}"
    try:
        subprocess.run([
            "/usr/local/bin/kubectl", "create", "deployment", deployment_name,
            "--image=nginx", "--replicas=2", "-n", "monsties"
        ], capture_output=True, text=True, check=True)
        return jsonify({'message': f'Monstie deployment {deployment_name} created successfully'}), 201
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500


@bp.route('/delete-deployment', methods=['POST'])
def delete_monstie_deployment():
    """Delete a monstie deployment."""
    data = request.json
    deployment_name = data.get('deployment-name')
    if not deployment_name:
        return jsonify({'error': 'Missing deployment-name in request'}), 400

    try:
        subprocess.run([
            "/usr/local/bin/kubectl", "delete", "deployment", deployment_name, "-n", "monsties"
        ], capture_output=True, text=True, check=True)
        return jsonify({'message': f'Monstie deployment {deployment_name} deleted successfully'}), 200
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500


@bp.route('/delete-deployment/<deployment_name>', methods=['DELETE'])
def delete_monstie_deployment_by_name(deployment_name):
    """Delete a monstie deployment using a URL parameter."""
    try:
        subprocess.run([
            "/usr/local/bin/kubectl", "delete", "deployment", deployment_name, "-n", "monsties"
        ], capture_output=True, text=True, check=True)
        return jsonify({'message': f'Monstie deployment {deployment_name} deleted successfully'}), 200
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500


@bp.route('/delete-pod', methods=['POST'])
def delete_monstie_pod():
    """Delete a monstie pod."""
    data = request.json
    pod_name = data.get('pod-name')
    if not pod_name:
        return jsonify({'error': 'Missing pod-name in request'}), 400

    try:
        subprocess.run([
            "/usr/local/bin/kubectl", "delete", "pod", pod_name, "-n", "monsties"
        ], capture_output=True, text=True, check=True)
        return jsonify({'message': f'Monstie pod {pod_name} deleted successfully'}), 200
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500


@bp.route('/delete-pod/<pod_name>', methods=['DELETE'])
def delete_monstie_pod_by_name(pod_name):
    """Delete a monstie pod using a URL parameter."""
    try:
        subprocess.run([
            "/usr/local/bin/kubectl", "delete", "pod", pod_name, "-n", "monsties"
        ], capture_output=True, text=True, check=True)
        return jsonify({'message': f'Monstie pod {pod_name} deleted successfully'}), 200
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500