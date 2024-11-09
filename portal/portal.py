from flask import Flask, request, jsonify, render_template
from prometheus_client import Gauge, Counter, generate_latest, CONTENT_TYPE_LATEST
import logging

app = Flask(__name__, template_folder="templates")

# Configure logging
logging.basicConfig(level=logging.INFO)

# Define Prometheus metrics for each numeric metric in PlayerMetrics
player_gold = Gauge('brogue_player_gold', 'Amount of gold collected by the player')
player_gold_generated = Gauge('brogue_gold_generated', 'Total amount of gold generated')
player_depth = Gauge('brogue_depth_level', 'Current depth level of the player')
player_deepest_level = Gauge('brogue_deepest_level', 'Deepest level reached by the player')
player_current_hp = Gauge('brogue_player_current_hp', 'Current hit points of the player')
player_max_hp = Gauge('brogue_player_max_hp', 'Maximum hit points of the player')
player_turns = Gauge('brogue_player_turns', 'Total turns played by the player')
player_strength = Gauge('brogue_strength', 'Player’s strength')
monster_spawn_fuse = Gauge('brogue_monster_spawn_fuse', 'Time until a monster spawns')

# Additional metrics from PlayerMetrics struct
player_turn_number = Gauge('brogue_player_turn_number', 'Number of turns taken by the player')
absolute_turn_number = Gauge('brogue_absolute_turn_number', 'Total turns since the beginning of the game')
milliseconds = Gauge('brogue_milliseconds_since_launch', 'Milliseconds since game launch')
xpxp_this_turn = Gauge('brogue_xpxp_this_turn', 'Squares explored this turn')
stealth_range = Gauge('brogue_stealth_range', 'Distance from which monsters will notice the player')
reward_rooms_generated = Gauge('brogue_reward_rooms_generated', 'Number of reward rooms generated')

# Ring bonuses
clairvoyance = Gauge('brogue_clairvoyance_bonus', 'Clairvoyance ring bonus')
stealth_bonus = Gauge('brogue_stealth_bonus', 'Stealth ring bonus')
regeneration_bonus = Gauge('brogue_regeneration_bonus', 'Regeneration ring bonus')
light_multiplier = Gauge('brogue_light_multiplier', 'Light multiplier ring bonus')
awareness_bonus = Gauge('brogue_awareness_bonus', 'Awareness ring bonus')
transference = Gauge('brogue_transference', 'Transference ring bonus')
wisdom_bonus = Gauge('brogue_wisdom_bonus', 'Wisdom ring bonus')
reaping = Gauge('brogue_reaping_bonus', 'Reaping ring bonus')

# Health and status effects
regen_per_turn = Gauge('brogue_regen_per_turn', 'HP regeneration per turn')
weakness_amount = Gauge('brogue_weakness_amount', 'Amount of weakness inflicted')
poison_amount = Gauge('brogue_poison_amount', 'Amount of poison inflicted')

# Monster counter
brogue_monster_count = Counter('brogue_monster_count', 'Total number of monsters created')
monster_death_count = Counter('brogue_monster_death_count', 'Total number of monsters that have died')

# Dictionaries to store monster data by ID
created_monsters = {}
dead_monsters = {}

# Global dictionaries to store player and game state data
game_state_data = {}
player_info_data = {}

# Endpoint to receive game metrics
@app.route('/metrics', methods=['POST'])
def receive_metrics():
    data = request.json
    app.logger.info("Received metrics data: %s", data) 
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    game_state_keys = [
        "RNG", "absoluteTurnNumber", "deepestLevel", "depthLevel", "easyMode",
        "gameHasEnded", "gameInProgress", "goldGenerated", "milliseconds",
        "monsterSpawnFuse", "playerTurnNumber", "rewardRoomsGenerated", "seed",
        "turns", "wizard"
    ]
    player_info_keys = [
        "awarenessBonus", "clairvoyance", "disturbed", "gold", "currentHP",
        "lightMultiplier", "maxHP", "poisonAmount", "reaping", "regenPerTurn",
        "regenerationBonus", "stealthBonus", "stealthRange", "strength",
        "transference", "weaknessAmount", "wisdomBonus", "xpXPThisTurn",
        "weapon", "armor", "ringLeft", "ringRight"
    ]

    game_state_data.update({key: data[key] for key in game_state_keys if key in data})
    player_info_data.update({key: data[key] for key in player_info_keys if key in data})
    player_info_data["inventory"] = data.get("inventory", [])

    return jsonify({"status": "success"}), 200

# Endpoint to receive or update monster data
@app.route('/monsters', methods=['POST'])
def monsters_data():
    monsters = request.json
    app.logger.info("Received monster data: %s", monsters)
    if not monsters:
        return jsonify({"error": "No JSON payload received"}), 400

    for monster in monsters:
        monster_id = monster.get("id")
        if monster_id is None:
            app.logger.warning("Skipping monster entry without 'id': %s", monster)
            continue

        # Check if monster is already created or updated
        if monster_id in created_monsters:
            if created_monsters[monster_id] != monster:
                created_monsters[monster_id].update(monster)
                app.logger.info("Updated existing monster: %s", monster)
        else:
            created_monsters[monster_id] = monster
            brogue_monster_count.inc()
            app.logger.info("Added new monster: %s", monster)

    return jsonify({"status": "success", "received": monsters}), 200

# Endpoint to receive monster death notifications
@app.route('/monster/death', methods=['POST'])
def receive_monster_death():
    data = request.json
    monster_id = data.get("id")
    if not data or monster_id is None:
        return jsonify({"error": "No valid JSON payload received"}), 400

    app.logger.info("Received death notification for monster ID: %s", monster_id)
    if monster_id in created_monsters:
        dead_monsters[monster_id] = created_monsters.pop(monster_id)
        monster_death_count.inc()

    return jsonify({"status": "success", "id": monster_id}), 200

# Endpoint to reset monster lists for a new game
@app.route('/monsters/reset', methods=['POST'])
def reset_monsters():
    created_monsters.clear()
    dead_monsters.clear()
    app.logger.info("Monster lists have been reset for a new game.")
    return jsonify({"status": "success"}), 200

# Other endpoints to serve player, game state, and monster data
@app.route('/player/data', methods=['GET'])
def player_data_endpoint():
    return jsonify(player_info_data)

@app.route('/gamestate/data', methods=['GET'])
def gamestate_data_endpoint():
    return jsonify(game_state_data)

@app.route('/monsters/data', methods=['GET'])
def monsters_data_endpoint():
    return jsonify({
        "created_monsters": list(created_monsters.values()),
        "dead_monsters": list(dead_monsters.values())
    })

# HTML page endpoints
@app.route('/', methods=['GET'])
def show_tracker():
    return render_template('tracker.html')

@app.route('/player', methods=['GET'])
def show_player():
    return render_template('player.html')

@app.route('/gamestate', methods=['GET'])
def show_gamestate():
    return render_template('gamestate.html')

@app.route('/monsters', methods=['GET'])
def show_monsters():
    return render_template('monsters.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
