from flask import Flask, request, jsonify, render_template
from prometheus_client import Gauge, Counter, generate_latest, CONTENT_TYPE_LATEST
import logging

app = Flask(__name__, template_folder="templates")

# Configure logging to see the monster details in the console
logging.basicConfig(level=logging.INFO)

# Define Prometheus metrics for each numeric metric in PlayerMetrics
player_gold = Gauge('brogue_player_gold', 'Amount of gold collected by the player')
player_gold_generated = Gauge('brogue_gold_generated', 'Total amount of gold generated')
player_depth = Gauge('brogue_depth_level', 'Current depth level of the player')
player_deepest_level = Gauge('brogue_deepest_level', 'Deepest level reached by the player')
player_hp = Gauge('brogue_player_current_hp', 'Current hit points of the player')
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

# Monster death counter
monster_death_count = Counter('brogue_monster_death_count', 'Total number of monsters that have died')

# Lists to store monster creation and death information
created_monsters = []
dead_monsters = []


# Endpoint to receive player metrics
@app.route('/metrics', methods=['POST', 'GET'])
def receive_metrics():
    if request.method == 'POST':
        data = request.json
        if not data:
            return jsonify({"error": "No JSON payload received"}), 400

        # Update numeric metrics with the received values
        player_gold.set(data.get('gold', 0))
        player_gold_generated.set(data.get('goldGenerated', 0))
        player_depth.set(data.get('depthLevel', 0))
        player_deepest_level.set(data.get('deepestLevel', 0))
        player_hp.set(data.get('hp', 0))
        player_turns.set(data.get('turns', 0))
        player_strength.set(data.get('strength', 0))
        monster_spawn_fuse.set(data.get('monsterSpawnFuse', 0))
        
        player_turn_number.set(data.get('playerTurnNumber', 0))
        absolute_turn_number.set(data.get('absoluteTurnNumber', 0))
        milliseconds.set(data.get('milliseconds', 0))
        xpxp_this_turn.set(data.get('xpxpThisTurn', 0))
        stealth_range.set(data.get('stealthRange', 0))
        reward_rooms_generated.set(data.get('rewardRoomsGenerated', 0))

        clairvoyance.set(data.get('clairvoyance', 0))
        stealth_bonus.set(data.get('stealthBonus', 0))
        regeneration_bonus.set(data.get('regenerationBonus', 0))
        light_multiplier.set(data.get('lightMultiplier', 0))
        awareness_bonus.set(data.get('awarenessBonus', 0))
        transference.set(data.get('transference', 0))
        wisdom_bonus.set(data.get('wisdomBonus', 0))
        reaping.set(data.get('reaping', 0))

        regen_per_turn.set(data.get('regenPerTurn', 0))
        weakness_amount.set(data.get('weaknessAmount', 0))
        poison_amount.set(data.get('poisonAmount', 0))

        return jsonify({"status": "success", "received": data}), 200
    elif request.method == 'GET':
        # Return the current values of each metric in JSON format
        return jsonify({
            "player_gold": player_gold._value.get(),
            "player_gold_generated": player_gold_generated._value.get(),
            "player_depth": player_depth._value.get(),
            "player_deepest_level": player_deepest_level._value.get(),
            "player_hp": player_hp._value.get(),
            "player_turns": player_turns._value.get(),
            "player_strength": player_strength._value.get(),
            "monster_spawn_fuse": monster_spawn_fuse._value.get(),
            "player_turn_number": player_turn_number._value.get(),
            "absolute_turn_number": absolute_turn_number._value.get(),
            "milliseconds": milliseconds._value.get(),
            "xpxp_this_turn": xpxp_this_turn._value.get(),
            "stealth_range": stealth_range._value.get(),
            "reward_rooms_generated": reward_rooms_generated._value.get(),
            "clairvoyance": clairvoyance._value.get(),
            "stealth_bonus": stealth_bonus._value.get(),
            "regeneration_bonus": regeneration_bonus._value.get(),
            "light_multiplier": light_multiplier._value.get(),
            "awareness_bonus": awareness_bonus._value.get(),
            "transference": transference._value.get(),
            "wisdom_bonus": wisdom_bonus._value.get(),
            "reaping": reaping._value.get(),
            "regen_per_turn": regen_per_turn._value.get(),
            "weakness_amount": weakness_amount._value.get(),
            "poison_amount": poison_amount._value.get(),
        }), 200

# Prometheus scrape endpoint
@app.route('/metrics/prometheus', methods=['GET'])
def prometheus_metrics():
    return generate_latest(), 200, {'Content-Type': CONTENT_TYPE_LATEST}

# Endpoint to receive monster creation data
@app.route('/monster', methods=['POST'])
def receive_monster():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    # Log monster data and store it
    app.logger.info("Received monster creation data: %s", data)
    created_monsters.append(data)  # Add to the created monsters list

    # Increment the monster count in Prometheus
    brogue_monster_count.inc()

    # Respond with success
    return jsonify({"status": "success", "received": data}), 200

# Endpoint to receive monster death notifications
@app.route('/monster/death', methods=['POST'])
def receive_monster_death():
    data = request.json
    if not data or 'portalName' not in data:
        return jsonify({"error": "No valid JSON payload received"}), 400

    app.logger.info("Received death notification for monster: %s", data['portalName'])
    app.logger.info("Monster death details: %s", data)
    dead_monsters.append(data)  # Add to the dead monsters list

    # Increment the death count in Prometheus
    monster_death_count.inc()

    # Respond with success
    return jsonify({"status": "success", "portalName": data['portalName']}), 200

# HTML page to display monster information
@app.route('/monsters', methods=['GET'])
def show_monsters():
    # Create a set of portal names for dead monsters to use for highlighting
    dead_monster_names = {monster['portalName'] for monster in dead_monsters}

    return render_template(
        'monsters.html',
        created_monsters=created_monsters,
        dead_monsters=dead_monsters,
        dead_monster_names=dead_monster_names
    )

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
