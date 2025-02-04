# fmt: off
"""
This module defines the `monsters` blueprint for managing monsters in the game.

It includes endpoints to manage monsters, track their status, and interact with Kubernetes
resources. 

This blueprint includes routes for:
- Viewing monsters' current data.
- Updating monsters (both creating new and updating existing monsters).
- Tracking the death of monsters.
- Resetting the game state related to monsters.
- Retrieving timestamps for monster spawn and death.

Returns:
    None: This module defines routes for the Flask app to manage monsters and Prometheus metrics.
"""
import json
from datetime import datetime, timezone
from app.models.monsters import Monster
from app.services.k8s_service import KubernetesService
from flask import Blueprint, current_app, jsonify, render_template, request
from kubernetes.client.exceptions import ApiException as KubernetesError
from prometheus_client import Counter, Gauge, Histogram
import requests
from flask_cors import CORS

bp = Blueprint('monsters', __name__)
CORS(bp)  # Enable CORS for the blueprint

# Define Prometheus metrics for tracking monsters
monster_count = Counter('brogue_monster_count',
                        'Total number of monsters created')
monster_death_count = Counter('brogue_monster_death_count',
                              'Total number of monsters that have died')

monster_lifespan_histogram = Histogram(
    'brogue_monster_lifespan_seconds',
    'Time (in seconds) monsters stay alive',
    # Time buckets for monitoring monster lifespan
    buckets=[10, 30, 60, 300, 600, 1800, 3600, 7200]
)

last_monster_created = Gauge(
    'brogue_last_monster_created_timestamp',
    'Timestamp of the last monster creation'
)
last_monster_death = Gauge(
    'brogue_last_monster_death_timestamp',
    'Timestamp of the last monster death'
)

active_monsters = {}
all_monsters = {}
dead_monsters = {}
admin_kills = {}

k8s_service = KubernetesService()


@bp.route("/", methods=["GET"])
def monsters():
    """
    Endpoint to render the monsters page.
    """
    return render_template("monsters.html")


@bp.route("/data", methods=["GET"])
def get_monsters():
    """
    Returns the list of all active monsters.

    This route retrieves all active monsters.

    Returns:
        Response: A JSON response containing the list of active monsters.
    """
    try:
        return jsonify([monster.dict() for monster in active_monsters.values()])
    except (AttributeError, KeyError, TypeError) as e:
        current_app.logger.error(f"Error fetching active monsters data: {e}")
        return jsonify({"error": "Error fetching data"}), 500


@bp.route("/count", methods=["GET"])
def get_monster_count():
    """
    Returns the count of live monsters.

    This route retrieves the number of monsters currently alive in the game.

    Returns:
        Response: A JSON response with the current live monster count.
    """
    count = len(active_monsters)
    return jsonify({"monster_count": count})


@bp.route("/dead-count", methods=["GET"])
def get_dead_monster_count():
    """
    Returns the count of dead monsters.

    This route retrieves the number of monsters that have died in the game.

    Returns:
        Response: A JSON response with the dead monster count.
    """
    count = len(dead_monsters)
    return jsonify({"dead_monster_count": count})


@bp.route("/monsties-count", methods=["GET"])
def get_monsties_count():
    """
    Returns the count of monsties.

    This is a placeholder route that currently always returns 0, presumably for tracking some 
    other monster-related metric.

    Returns:
        Response: A JSON response with the count of monsties.
    """
    count = 0
    return jsonify({"monsties_count": count})


@bp.route("/all", methods=["GET"])
def get_all_monsters():
    """
    Returns all the monsters, alive and dead.

    This route retrieves all monsters, including both live and dead, stored in memory.

    Returns:
        Response: A JSON response containing all monsters, both alive and dead.
    """
    return jsonify([monster.dict() for monster in all_monsters.values()])


@bp.route("/dead", methods=["GET"])
def get_dead_monsters():
    """
    Returns a list of dead monsters.

    This route retrieves all monsters that are marked as dead.

    Returns:
        Response: A JSON response containing the list of dead monsters.
    """
    return jsonify([monster.dict() for monster in dead_monsters.values()])


@bp.route("/admin-kills", methods=["GET"])
def get_admin_kill_monsters():
    """
    Returns a list of admin kill monsters.

    This route retrieves all monsters that are marked as admin kill.

    Returns:
        Response: A JSON response containing the list of admin kill monsters.
    """
    try:
        #current_app.logger.info(f"Admin kills: {admin_kills}")
        return jsonify([kill for kill in admin_kills.values()])
    except (AttributeError, KeyError, TypeError) as e:
        current_app.logger.error(f"Error fetching admin kill monsters data: {e}")
        return jsonify({"error": "Error fetching data"}), 500


@bp.route("/admin-kills/<int:monster_id>", methods=["GET"], strict_slashes=False)
def is_admin_kill(monster_id):
    """
    Checks if a monster is in the admin_kills list.

    Args:
        monster_id (int): The ID of the monster to check.

    Returns:
        Response: A JSON response indicating if the monster is in the admin_kills list.
    """
    is_id_admin_kill = monster_id in admin_kills
    return jsonify({"is_admin_kill": is_id_admin_kill})


@bp.route("/timestamps", methods=["GET"], strict_slashes=False)
def get_monster_timestamps():
    """
    Handles the GET request to retrieve the spawn and death timestamps for all monsters.

    This function iterates over the `all_monsters` collection, extracting the `name`, 
    `spawnTimestamp`, and `deathTimestamp` for each monster. It then returns a JSON response 
    containing a list of these timestamps. If either timestamp is missing, it returns "Unknown" 
    as a placeholder value.

    Logging:
    - Logs the number of monsters whose timestamps were retrieved.
    - Logs an error message if an exception occurs during the retrieval process.

    Returns:
        Response: A JSON response containing a list of dictionaries with the monster names and their
            respective timestamps (spawn and death), or an error message if an exception is raised.

    Exceptions:
        - ValueError, TypeError: These exceptions are caught and logged, and an error response is 
        returned with a 500 status code if an issue occurs during the process.
    """
    try:
        timestamps = []
        for monster in all_monsters.values():
            timestamps.append({
                "name": monster.name,
                "spawnTimestamp":
                    monster.spawn_timestamp.isoformat() if monster.spawn_timestamp else "Unknown",
                "deathTimestamp":
                    monster.death_timestamp.isoformat() if monster.death_timestamp else "Unknown",
            })

        return jsonify(timestamps)

    except (ValueError, TypeError) as e:
        current_app.logger.error(f"Error retrieving timestamps: {e}")
        return jsonify({"error": "Error retrieving timestamps"}), 500


@bp.route("/update", methods=["POST"], strict_slashes=False)
def create():
    """
    Creates or updates monster data.

    This route accepts a POST request with monster data, validates it using the `Monster` model,
    and either creates a new monster or updates an existing monster's information. Also tracks 
    monster creation in Kubernetes.

    Returns:
        Response: A JSON response with the status of the request, including the updated monster
        data.
    """
    received_data = request.json
    if not received_data:
        return jsonify({"error": "No JSON payload received"}), 400

    for monster_data in received_data:
        try:
            monster = Monster(**monster_data)
            current_app.logger.info(f"Monster data validated: {monster}")
        except (ValueError, TypeError) as e:
            current_app.logger.error(f"Error validating monster data: {e}")
            return jsonify({"error": "Invalid monster data", "message": str(e)}), 400

        monster_id = monster.id
        if monster_id is None:
            current_app.logger.warning(f"Skipping monster entry without 'id': {monster_data}")
            continue

        if monster_id not in all_monsters:
            return_value = handle_new_monster(monster)
            if return_value:
                return return_value
        else:
            handle_existing_monster(monster)

        update_monster_status(monster)

    return jsonify({"status": "success", "message": "monster update data received"}), 200


def handle_new_monster(monster: Monster):
    """
    Handle the creation of a new monster in the game.

    Args:
        monster: The Monster object that was validated.

    Returns:
        None: If successful, the function returns None. If an error occurs, an error 
            response is returned.
    """
    monster_count.inc()
    if not monster.spawn_timestamp:
        monster.spawn_timestamp = datetime.now(timezone.utc)

    current_app.logger.info(f"Adding new monster: {monster.name}")
    current_app.logger.info(f"Monster data: {monster.model_dump()}")
    all_monsters[monster.id] = monster

    try:
        k8s_service.create_monster_resource(
            name=monster.name,
            namespace="dungeon-master-system",
            monster_data=monster.model_dump()
        )
        current_app.logger.info(f"Monster resource created for {monster.name}.")
    except KubernetesError as e:
        current_app.logger.error(f"Failed to create Monster resource for {monster.name}: {e}")
        return jsonify({"error": "Failed to create Monster resource", "message": str(e)}), 500

    return None


def handle_existing_monster(monster: Monster):
    """
    Handle updating an existing monster's data.

    This function checks if the monster is alive before attempting to update its resource
    in Kubernetes. If the monster is dead, the update is skipped.

    Args:
        monster: The Monster object containing the updated data.
        monster_id: The ID of the monster to update.

    Returns:
        None: If the update is successful, nothing is returned. If an error occurs, an error
        response is returned.
    """
    if not monster.is_dead:
        try:
            k8s_service.update_monster_resource(
                name=monster.name,
                namespace="dungeon-master-system",
                monster_data=monster.model_dump()
            )
            current_app.logger.info(f"Monster resource updated for {monster.name}.")
        except KubernetesError as e:
            current_app.logger.error(f"Failed to update Monster resource for {monster.name}: {e}")
            return jsonify({"error": "Failed to update Monster resource", "message": str(e)}), 500


def update_monster_status(monster: Monster):
    """
    Update the status of a monster, either adding it to the live monsters or marking it as dead.

    Args:
        monster: The Monster object whose status needs to be updated.
        monster_id: The ID of the monster to update.

    Returns:
        None
    """
    if not monster.spawn_timestamp:
        monster.spawn_timestamp = datetime.now(timezone.utc)
    if not monster.is_dead:
        active_monsters[monster.id] = monster
        all_monsters[monster.id] = monster
    else:
        monster.death_timestamp = datetime.now(timezone.utc)
        dead_monsters[monster.id] = monster


@bp.route("/death", methods=["POST"], strict_slashes=False)
def receive_monster_death():
    """
    Handles the death of a monster and updates the corresponding data.

    Args:
        None.

    Returns:
        Response: A JSON response indicating the success or failure of the operation.
    """
    data = request.json

    monster_id = data.get("id")
    if not data or monster_id is None:
        return jsonify({"error": "No valid JSON payload received"}), 400

    if isinstance(monster_id, str):
        try:
            monster_id = int(monster_id)
        except ValueError:
            return jsonify({"error": f"Invalid monster ID: {monster_id}"}), 400

    if monster_id not in all_monsters:
        return jsonify({"error": f"Monster with ID {monster_id} not found"}), 404

    monster = all_monsters[monster_id]
    if monster.is_dead:
        return jsonify({"error": f"Monster with ID {monster.id} is already dead."}), 400

    # Mark monster as dead
    monster.is_dead = True
    monster.death_timestamp = datetime.now(timezone.utc)

    # Update the Prometheus metrics
    monster_death_count.inc()
    active_monsters.pop(monster.id, None)
    dead_monsters[monster_id] = monster

    # Log the updated monster status
    current_app.logger.info(f"Monster marked as dead: {monster.name}, ID: {monster.id}")

    # Call the Kubernetes service to delete the monster resource
    try:
        k8s_service.delete_monster_resource(
            name=monster.name, namespace="dungeon-master-system"
        )
        current_app.logger.info(f"Successfully deleted Monster resource: {monster.name}")
    except KubernetesError as e:
        current_app.logger.error(f"Failed to delete Monster resource: {monster.name} due to {e}")
        return jsonify({"error": f"Failed to delete Monster resource: {monster.name}"}), 500

    return jsonify({"status": "success", "id": monster.id}), 200


@bp.route('/reset', methods=['POST'], strict_slashes=False)
def reset_current_game_monsters():
    """
    Resets the current game state for monsters.

    Clears the in-memory data for live and dead monsters, and deletes all associated Kubernetes 
    resources.

    Returns:
        Response: A JSON response indicating the status of the reset.
    """
    active_monsters.clear()
    all_monsters.clear()
    dead_monsters.clear()
    admin_kills.clear()  # Reset the admin_kills list

    k8s_service.delete_all_monsters_in_namespace("dungeon-master-system")

    current_app.logger.info("Monster data has been reset for a new game.")
    return jsonify({"status": "success"}), 200


def sanitize_string(input_str):
    """
    Sanitizes a string by removing control characters.

    Args:
        Input string with control characters to be removed.

    Returns:
       Sanitized string with control characters removed.
    """
    if isinstance(input_str, str):
        # Replace control characters with safe placeholders
        return ''.join(ch for ch in input_str if ch.isprintable())
    return input_str


@bp.route('/admin-kill', methods=['POST'], strict_slashes=False)
def admin_kill():
    """
    Receives a notice that a monster has been admin killed.
    Relays the information to the game.

    Returns:
        Response: A JSON response indicating the status of the notice and relay.
    """
    try:
        # Log raw incoming data for debugging
        raw_data = request.data.decode('utf-8')

        data = json.loads(raw_data, strict=False)
        if not data:
            current_app.logger.warning("No JSON payload received.")
            return jsonify({"error": "No JSON payload received"}), 400

        monster_name = sanitize_string(data.get("monsterName", "Unknown"))
        pod_name = sanitize_string(data.get("podName", "Unknown"))
        monster_id = sanitize_string(data.get("monsterID", "Unknown"))
        namespace = sanitize_string(data.get("namespace", "Unknown"))
        depth = sanitize_string(data.get("depth", "Unknown"))

        # Log the received admin kill notification
        current_app.logger.info(f"Admin kill notice: Name={monster_name}, PodName={pod_name}, ID={monster_id}, Namespace={namespace}, Depth={depth}")

        # Check if the monster is in the active_monsters list
        if monster_id in active_monsters:
            # Add the monster to the admin_kills list if it isn't already there
            if monster_id not in admin_kills:
                admin_kills[monster_id] = {
                    "monster_name": monster_name,
                    "pod_name": pod_name,
                    "monster_id": monster_id,
                    "namespace": namespace,
                    "depth": depth,
                    "timestamp": datetime.now(timezone.utc).isoformat()
                }
                current_app.logger.info(f"Added to admin_kills list: {monster_name}, PodName={pod_name}, ID={monster_id}, Namespace={namespace}, Depth={depth}")

            # Move the monster from active_monsters to dead_monsters
            monster = active_monsters.pop(monster_id)
            monster.is_dead = True
            monster.death_timestamp = datetime.now(timezone.utc)
            dead_monsters[monster_id] = monster
            current_app.logger.info(f"Monster marked as dead: {monster.name}, ID: {monster_id}")
        else:
            current_app.logger.warning(f"Monster with ID {monster_id} not found in active_monsters list.")

        # Log the current state of the lists
        #current_app.logger.info(f"Admin kills: {[monster for monster in admin_kills.values()]}")

        return jsonify({"message": "Admin kill notice processed"}), 200

    except json.JSONDecodeError as e:
        current_app.logger.error(f"JSON decoding error: {e}")
        return jsonify({"error": "Invalid JSON payload", "details": str(e)}), 400

    except requests.Timeout:
        current_app.logger.error("Request to game timed out.")
        return jsonify({"error": "Request to game timed out"}), 504

    except requests.RequestException as e:
        current_app.logger.error(f"RequestException: {e}")
        return jsonify({"error": "Failed to relay admin kill notice", "details": str(e)}), 500

    except (ValueError, TypeError, KeyError) as e:
        current_app.logger.error(f"Error processing admin kill notice: {e}")
        return jsonify({"error": "Failed to process admin kill notice", "details": str(e)}), 500

@bp.route('/admin-kill/<int:monster_id>', methods=['DELETE'], strict_slashes=False)
def admin_kill_monster_by_id(monster_id):
    """
    Kills a monster by ID.

    Args:
        monster_id (int): The ID of the monster to kill.

    Returns:
        Response: A JSON response indicating the status of the kill operation.
    """
    if monster_id in active_monsters:
        if monster_id not in admin_kills:
            admin_kills[monster_id] = {
                "monster_name": active_monsters[monster_id].monster_name,
                "pod_name": active_monsters[monster_id].pod_name,
                "monster_id": monster_id,
                "namespace": active_monsters[monster_id].namespace,
                "depth": active_monsters[monster_id].depth,
                "timestamp": datetime.now(timezone.utc).isoformat()
            }
        monster = active_monsters.pop(monster_id)
        monster.is_dead = True
        monster.death_timestamp = datetime.now(timezone.utc)
        dead_monsters[monster_id] = monster
        current_app.logger.info(f"Monster admin killed: {monster.name}, ID: {monster_id}")
        return jsonify({"status": "success", "message": "Monster admin killed"}), 200

    return jsonify({"error": f"Monster with ID {monster_id} not found"}), 404