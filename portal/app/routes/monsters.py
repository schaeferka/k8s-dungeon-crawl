from flask import Blueprint, jsonify, render_template, request, current_app
from app.models.monsters import Monster  # Assuming you have a Monster model
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
import time
from app.services.kubernetes import KubernetesService

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

# In-memory storage for monsters (or use a database in real applications)
monsters_data = {}  # Using a dictionary instead of a list
monsters_overall_data = {}  # Using a dictionary for overall monsters
monsters_dead = []  # List for dead monsters

@bp.route('/', methods=['GET'])
def monsters():
    """
    Displays the monsters page.
    """
    return render_template('monsters.html')

# Route for getting all monsters
@bp.route('/data', methods=['GET'])
def get_monsters():
    """
    Returns the list of all monsters.
    """
    return jsonify(list(monsters_data.values()))  # Convert dict to list

# Route for getting the count of all monsters
@bp.route('/count', methods=['GET'])
def get_monster_count():
    """
    Returns the total count of monsters.
    """
    count = len(monsters_data)
    return jsonify({"monster_count": count})

# Route for getting all monsters (overall, same as /)
@bp.route('/overall', methods=['GET'])
def get_all_monsters():
    """
    Returns all the monsters.
    """
    return jsonify(list(monsters_overall_data.values()))  # Convert dict to list

# Route for getting only dead monsters
@bp.route('/dead', methods=['GET'])
def get_dead_monsters():
    """
    Returns a list of dead monsters.
    """
    return jsonify(monsters_dead)

# Route for getting monster creation/death timestamps
@bp.route('/timestamps', methods=['GET'], strict_slashes=False)
def get_monster_timestamps():
    """
    Returns the timestamps of when monsters were spawned or died.
    """
    timestamps = [{"name": monster["name"], "spawned_at": monster["spawnTimestamp"], "died_at": monster["deathTimestamp"]} for monster in monsters_overall_data.values()]
    return jsonify(timestamps)

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

        # Record the creation time if this is a new monster
        if monster_id not in monsters_overall_data:
            monster["creation_time"] = time.time()
            monster_count.inc()
            current_app.logger.info("Added new monster to overall monsters: %s", monster)

        # Update or add the monster in `monsters_overall_data`
        monsters_overall_data[monster_id] = {**monsters_overall_data.get(monster_id, {}), **monster}

        # Create the Monster custom resource in Kubernetes when a new monster is added
        try:
            k8s_service.create_monster_resource(
                name=monster["name"],
                namespace="default",  # Specify the correct namespace here
                monster_data=monster
            )
        except Exception as e:
            current_app.logger.error(f"Failed to create Monster resource for {monster_id}: {e}")
            
            
        # Ensure `monsters_data` has the latest data for active monsters
        if not monster.get("isDead", False):
            monsters_data[monster_id] = monster
        else:
            # If the monster is dead, add it to the dead list
            monsters_dead.append(monster)

    return jsonify({"status": "success", "received": data}), 200


@bp.route("/death", methods=["POST"], strict_slashes=False)
def receive_monster_death():
    """
    Handles the death of a monster and updates the corresponding data.
    """
    data = request.json

    # Ensure that the required data is present in the request
    monster_id = data.get("id")
    if not data or monster_id is None:
        return jsonify({"error": "No valid JSON payload received"}), 400

    current_app.logger.info("Received death notification for monster ID: %s", monster_id)

    # Check if the monster exists in the overall data
    if monster_id in monsters_overall_data:
        # Calculate lifespan if the monster exists in the overall data
        creation_time = monsters_overall_data[monster_id].get("creation_time")
        if creation_time:
            lifespan = time.time() - creation_time
            monster_lifespan_histogram.observe(lifespan)

        # Record the last monster death time
        last_monster_death.set_to_current_time()

        # Update the monster's stats to reflect its death
        monsters_data[monster_id]["hp"] = 0
        monsters_data[monster_id]["isDead"] = True
        monsters_overall_data[monster_id]["hp"] = 0
        monsters_overall_data[monster_id]["isDead"] = True

        # Move the monster to the 'dead' collection
        monsters_dead.append(monsters_overall_data[monster_id])

        # Increment the monster death count in Prometheus
        monster_death_count.inc()

        # Optionally, remove from active monsters list if no longer relevant
        monsters_data.pop(monster_id, None)
        
        # Delete the Monster custom resource in Kubernetes when the monster dies
        try:
            k8s_service.delete_monster_resource(
                name=monster_id,
                namespace="default"  # Specify the correct namespace here
            )
        except Exception as e:
            current_app.logger.error(f"Failed to delete Monster resource for {monster_id}: {e}")

        # Return a success response with the monster ID
        return jsonify({"status": "success", "id": monster_id}), 200
    else:
        # Handle the case where the monster ID doesn't exist in the overall data
        return jsonify({"error": f"Monster with ID {monster_id} not found"}), 404
    

@bp.route('/reset', methods=['POST'], strict_slashes=False)
def reset_current_game_monsters():
    monsters_data.clear()  # Clear the current game monsters list
    monsters_dead.clear()  # Clear the dead monsters list as well
    current_app.logger.info("Current game monster data and dead monsters data have been reset for a new game.")
    return jsonify({"status": "success"}), 200
