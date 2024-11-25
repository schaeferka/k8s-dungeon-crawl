from flask import Blueprint, jsonify, render_template, request, current_app
from app.models.monsters import Monster
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
from app.services.kubernetes import KubernetesService
from datetime import datetime
from time import time

bp = Blueprint('monsters', __name__)

k8s_service = KubernetesService()

# Define Prometheus metrics for monsters
monster_count = Counter('brogue_monster_count', 'Total number of monsters created')
monster_death_count = Counter('brogue_monster_death_count', 'Total number of monsters that have died')

monster_lifespan_histogram = Histogram(
    'brogue_monster_lifespan_seconds', 
    'Time (in seconds) monsters stay alive',
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

# TODO: Implement database storage for monsters
monsters_data = {}
monsters_all_data = {}
monsters_dead = []

@bp.route('/', methods=['GET'])
def monsters():
    """
    Displays the monsters page.
    """
    return render_template('monsters.html')

@bp.route('/data', methods=['GET'])
def get_monsters():
    """
    Returns the list of all monsters.
    """
    return jsonify(list(monsters_data.values()))

@bp.route('/count', methods=['GET'])
def get_monster_count():
    """
    Returns the count of live monsters.
    """
    count = len(monsters_data)
    return jsonify({"monster_count": count})

@bp.route('/dead-count', methods=['GET'])
def get_dead_monster_count():
    """
    Returns the count of dead monsters.
    """
    count = len(monsters_dead)
    return jsonify({"dead_monster_count": count})

@bp.route('/monsties-count', methods=['GET'])
def get_monsties_count():
    """
    Returns the count of monsties.
    """
    count = 0
    return jsonify({"monsties_count": count})

@bp.route('/all', methods=['GET'])
def get_all_monsters():
    """
    Returns all the monsters.
    """
    return jsonify(list(monsters_all_data.values())) 

@bp.route('/dead', methods=['GET'])
def get_dead_monsters():
    """
    Returns a list of dead monsters.
    """
    return jsonify(monsters_dead)

@bp.route('/timestamps', methods=['GET'], strict_slashes=False)
def get_monster_timestamps():
    """
    Returns the timestamps of when monsters were spawned or died.
    """
    try:
        timestamps = []
        for monster in monsters_all_data.values():
            if "name" in monster and "spawnTimestamp" in monster and "deathTimestamp" in monster:
                timestamps.append({
                    "name": monster["name"],
                    "spawnTimestamp": monster["spawnTimestamp"],
                    "deathTimestamp": monster["deathTimestamp"]
                })
            else:
                timestamps.append({
                    "name": monster.get("name", "Unknown"),
                    "spawnTimestamp": monster.get("spawnTimestamp", "Unknown"),
                    "deathTimestamp": monster.get("deathTimestamp", "Unknown")
                })

        return jsonify(timestamps)

    except Exception as e:
        current_app.logger.error(f"Error retrieving timestamps: {e}")
        return jsonify({"error": "Internal server error"}), 500


@bp.route("/update", methods=["POST"], strict_slashes=False) 
def create():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    for monster in data:
        monster_id = monster.get("id")
        if monster_id is None:
            current_app.logger.warning("Skipping monster entry without 'id': %s", monster)
            continue

        monster["spawnTimestamp"] = int(time()) 

        if monster_id not in monsters_all_data:
            return_value = handle_new_monster(monster)
            if return_value:
                return return_value  
        else:
            handle_existing_monster(monster, monster_id)

        update_monster_status(monster, monster_id)

    return jsonify({"status": "success", "received": data}), 200

def handle_new_monster(monster):
    monster_id = monster["id"]
    
    monster_name = monster["name"]
    if not monster_name:
        monster_name = "unknown-monster"  # Fallback name if empty

    monster_count.inc()
    current_app.logger.info("Added new monster to all monsters: %s", monster)
    monsters_all_data[monster_id] = {**monsters_all_data.get(monster_id, {}), **monster}
    
    try:
        current_app.logger.info("Creating Monster resource for monster ID: %s", monster_id)
        k8s_service.create_monster_resource(
            name=monster["name"],
            namespace="dungeon-master-system", 
            monster_data=monster
        )
        current_app.logger.info(f"Monster resource created for {monster_id}.")
    except Exception as e:
        current_app.logger.error(f"Failed to create Monster resource for {monster_id}: {e}")
        return jsonify({"error": "Failed to create Monster resource", "message": str(e)}), 500

    return None 

def handle_existing_monster(monster, monster_id):
    if not monster.get("isDead", False):
        try:
            current_app.logger.info(f"Updating Monster resource for monster ID: {monster_id}")
            k8s_service.update_monster_resource(
                name=monster["name"],
                namespace="dungeon-master-system", 
                monster_data=monster
            )
            current_app.logger.info(f"Monster resource updated for {monster_id}.")
        except Exception as e:
            current_app.logger.error(f"Failed to update Monster resource for {monster_id}: {e}")
            return jsonify({"error": "Failed to update Monster resource", "message": str(e)}), 500
    else:
        current_app.logger.info(f"Monster {monster_id} is dead, skipping update.")
        
        
def update_monster_status(monster, monster_id):
    if not monster.get("isDead", False):
        monsters_data[monster_id] = monster
        monsters_all_data[monster_id] = monster
    else:
        monsters_dead.append(monster)


@bp.route("/death", methods=["POST"], strict_slashes=False)
def receive_monster_death():
    """
    Handles the death of a monster and updates the corresponding data.
    """
    data = request.json

    current_app.logger.info("Received data for a death notification: %s", data)

    monster_id = data.get("id")
    if not data or monster_id is None:
        return jsonify({"error": "No valid JSON payload received"}), 400
    
    if isinstance(monster_id, str):
        try:
            monster_id = int(monster_id)
        except ValueError:
            return jsonify({"error": f"Invalid monster ID: {monster_id}"}), 400

    current_app.logger.info("Received death notification for monster ID: ", monster_id)

    if monster_id not in monsters_all_data:
        return jsonify({"error": f"Monster with ID {monster_id} not found"}), 404

    monster = monsters_all_data[monster_id]
    dead_monster = monster["name"] 
    if monster.get("isDead", False):
        current_app.logger.info(f"Monster with ID {monster_id} is already dead.")
        return jsonify({"error": f"Monster with ID {monster_id} is already dead."}), 400

    creation_time = monster.get("creation_time")
    if creation_time:
        lifespan = time.time() - creation_time
        monster_lifespan_histogram.observe(lifespan)

    current_app.logger.info(f"Marking monster {dead_monster} as dead.")
    monster["isDead"] = True
    monsters_dead.append(monsters_all_data[monster_id])
    monster_death_count.inc()
    monsters_data.pop(monster_id, None)

    # Delete the Monster custom resource in Kubernetes when the monster dies
    try:
        k8s_service.delete_monster_resource(
            name=monster['name'], 
            namespace="dungeon-master-system"
        )
    except Exception as e:
        current_app.logger.error(f"Failed to delete Monster resource for {monster_id}: {e}")
        return jsonify({"error": f"Failed to delete Monster resource: {monster_id}"}), 500

    return jsonify({"status": "success", "id": monster_id}), 200


@bp.route('/reset', methods=['POST'], strict_slashes=False)
def reset_current_game_monsters():
    monsters_data.clear() 
    monsters_all_data.clear()
    monsters_dead.clear()
    
    k8s_service.delete_all_monsters_in_namespace("dungeon-master-system")
    
    current_app.logger.info("Current game monster data and dead monsters data have been reset for a new game.")
    return jsonify({"status": "success"}), 200
