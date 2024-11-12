from flask import Flask, request, jsonify, render_template
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST
import logging
import time

app = Flask(__name__, template_folder="templates")

# Configure logging
logging.basicConfig(level=logging.INFO)

# Prometheus Gauges and Counters for PlayerMetrics, ItemMetrics, and GameStateMetrics

# Player Metrics
player_gold = Gauge('brogue_player_gold', 'Amount of gold collected by the player')
player_depth = Gauge('brogue_depth_level', 'Current depth level of the player')
player_deepest_level = Gauge('brogue_deepest_level', 'Deepest level reached by the player')
player_current_hp = Gauge('brogue_player_current_hp', 'Current hit points of the player')
player_max_hp = Gauge('brogue_player_max_hp', 'Maximum hit points of the player')
player_strength = Gauge('brogue_strength', 'Player’s strength')
player_stealth_range = Gauge('brogue_stealth_range', 'Distance from which monsters will notice the player')
player_turns = Gauge('brogue_player_turns', 'Total turns played by the player')
player_regen_per_turn = Gauge('brogue_regen_per_turn', 'HP regeneration per turn')
player_weakness_amount = Gauge('brogue_weakness_amount', 'Amount of weakness inflicted')
player_poison_amount = Gauge('brogue_poison_amount', 'Amount of poison inflicted')

# Bonuses and modifiers from rings
bonus_clairvoyance = Gauge('brogue_clairvoyance_bonus', 'Clairvoyance ring bonus')
bonus_stealth = Gauge('brogue_stealth_bonus', 'Stealth ring bonus')
bonus_regeneration = Gauge('brogue_regeneration_bonus', 'Regeneration ring bonus')
bonus_light_multiplier = Gauge('brogue_light_multiplier', 'Light multiplier ring bonus')
bonus_awareness = Gauge('brogue_awareness_bonus', 'Awareness ring bonus')
bonus_transference = Gauge('brogue_transference', 'Transference ring bonus')
bonus_wisdom = Gauge('brogue_wisdom_bonus', 'Wisdom ring bonus')
bonus_reaping = Gauge('brogue_reaping_bonus', 'Reaping ring bonus')

# Game State Metrics
game_reward_rooms = Gauge('brogue_reward_rooms_generated', 'Number of reward rooms generated')
game_has_ended = Gauge('brogue_game_has_ended', 'Indicates if the game has ended')
game_monster_spawn_fuse = Gauge('brogue_monster_spawn_fuse', 'Time until a monster spawns')
game_gold_generated = Gauge('brogue_gold_generated', 'Total amount of gold generated')
game_absolute_turn_number = Gauge('brogue_absolute_turn_number', 'Total turns since the beginning of the game')
game_milliseconds = Gauge('brogue_milliseconds_since_launch', 'Milliseconds since game launch')
game_xpxp_this_turn = Gauge('brogue_xpxp_this_turn', 'Squares explored this turn')

# Prometheus Gauges for Game Stats
games_played = Gauge('brogue_games_played', 'Total games played')
games_escaped = Gauge('brogue_games_escaped', 'Total games escaped')
games_mastered = Gauge('brogue_games_mastered', 'Total games mastered')
games_won = Gauge('brogue_games_won', 'Total games won')
win_rate = Gauge('brogue_win_rate', 'Win rate as a percentage')
deepest_level = Gauge('brogue_game_deepest_level', 'Deepest level reached')
cumulative_levels = Gauge('brogue_cumulative_levels', 'Total cumulative levels reached across all games')
highest_score = Gauge('brogue_highest_score', 'Highest score achieved')
cumulative_score = Gauge('brogue_cumulative_score', 'Cumulative score across all games')
most_gold = Gauge('brogue_most_gold', 'Most gold collected in a single game')
cumulative_gold = Gauge('brogue_cumulative_gold', 'Cumulative gold collected across all games')
most_lumenstones = Gauge('brogue_most_lumenstones', 'Most lumenstones collected in a single game')
cumulative_lumenstones = Gauge('brogue_cumulative_lumenstones', 'Cumulative lumenstones collected across all games')
fewest_turns_win = Gauge('brogue_fewest_turns_win', 'Fewest turns taken to win a game')
cumulative_turns = Gauge('brogue_cumulative_turns', 'Cumulative turns played across all games')
longest_win_streak = Gauge('brogue_longest_win_streak', 'Longest win streak')
longest_mastery_streak = Gauge('brogue_longest_mastery_streak', 'Longest mastery streak')
current_win_streak = Gauge('brogue_current_win_streak', 'Current win streak')
current_mastery_streak = Gauge('brogue_current_mastery_streak', 'Current mastery streak')

# Item Metrics
item_weapon_damage_min = Gauge('brogue_weapon_damage_min', 'Minimum damage of equipped weapon')
item_weapon_damage_max = Gauge('brogue_weapon_damage_max', 'Maximum damage of equipped weapon')
item_armor_defense = Gauge('brogue_armor_defense', 'Defense of equipped armor')

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

# Dictionaries to store monster data by ID
current_game_monsters = {}
overall_monsters = {}  # Maintains data across games
dead_monsters = {}     # Records deaths across games

# Global dictionaries to store player and game state data
game_state_data = {}
player_info_data = {}
items_data = {}
game_stats_data = {}
player_pack_data = {}

# Endpoint to serve Prometheus metrics in text format (GET)
@app.route('/metrics', methods=['GET'])
def metrics():
    return generate_latest(), 200, {'Content-Type': CONTENT_TYPE_LATEST}

# Endpoint to receive Player Metrics (POST)
@app.route('/player', methods=['POST'])
def receive_player_metrics():
    data = request.json
    app.logger.info("Received player metrics data: %s", data)
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    # Populate the player_info_data dictionary
    player_info_data.update(data)

    # Set Prometheus Gauges only if data fields are present
    if "gold" in data:
        player_gold.set(data["gold"])
    if "depthLevel" in data:
        player_depth.set(data["depthLevel"])
    if "currentHP" in data:
        player_current_hp.set(data["currentHP"])
    if "maxHP" in data:
        player_max_hp.set(data["maxHP"])
    if "strength" in data:
        player_strength.set(data["strength"])
    if "stealthRange" in data:
        player_stealth_range.set(data["stealthRange"])
    if "regenPerTurn" in data:
        player_regen_per_turn.set(data["regenPerTurn"])
    if "weaknessAmount" in data:
        player_weakness_amount.set(data["weaknessAmount"])
    if "poisonAmount" in data:
        player_poison_amount.set(data["poisonAmount"])

    return jsonify({"status": "success"}), 200

@app.route('/pack', methods=['POST'])
def receive_pack_data():
    data = request.json
    app.logger.info("Received pack data: %s", data)

    if not data or "pack" not in data:
        return jsonify({"error": "Invalid or missing pack data"}), 400

    # Optional: Validate structure of each item in the pack
    for item in data.get("pack", []):
        if not all(k in item for k in ["category", "name", "quantity", "inventoryLetter"]):
            return jsonify({"error": "Item missing required fields"}), 400

    # Store the pack data
    player_pack_data.clear()
    player_pack_data.update(data)
    
    return jsonify({"status": "success", "received": data}), 200

# Endpoint to receive Game State Metrics (POST)
@app.route('/gamestate', methods=['POST'])
def receive_game_state_metrics():
    data = request.json
    app.logger.info("Received game state metrics data: %s", data)
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    # Populate the game_state_data dictionary
    game_state_data.update(data)

    # Set Prometheus Gauges only if data fields are present
    if "rewardRoomsGenerated" in data:
        game_reward_rooms.set(data["rewardRoomsGenerated"])
    if "monsterSpawnFuse" in data:
        game_monster_spawn_fuse.set(data["monsterSpawnFuse"])
    if "goldGenerated" in data:
        game_gold_generated.set(data["goldGenerated"])
    if "absoluteTurnNumber" in data:
        game_absolute_turn_number.set(data["absoluteTurnNumber"])
    if "milliseconds" in data:
        game_milliseconds.set(data["milliseconds"])
    if "xpxpThisTurn" in data:
        game_xpxp_this_turn.set(data["xpxpThisTurn"])
    if "gameHasEnded" in data:
        game_has_ended.set(1 if data["gameHasEnded"] else 0)

    return jsonify({"status": "success"}), 200

@app.route('/gamestats', methods=['POST'])
def receive_game_stats_metrics():
    data = request.json
    app.logger.info("Received game stats metrics data: %s", data)
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    # Update the game_stats_data dictionary
    game_stats_data.update(data)

    # Set Prometheus Gauges only if data fields are present
    games_played.set(data.get("games", 0))
    games_escaped.set(data.get("escaped", 0))
    games_mastered.set(data.get("mastered", 0))
    games_won.set(data.get("won", 0))
    win_rate.set(data.get("winRate", 0.0))
    deepest_level.set(data.get("deepestLevel", 0))
    cumulative_levels.set(data.get("cumulativeLevels", 0))
    highest_score.set(data.get("highestScore", 0))
    cumulative_score.set(data.get("cumulativeScore", 0))
    most_gold.set(data.get("mostGold", 0))
    cumulative_gold.set(data.get("cumulativeGold", 0))
    most_lumenstones.set(data.get("mostLumenstones", 0))
    cumulative_lumenstones.set(data.get("cumulativeLumenstones", 0))
    fewest_turns_win.set(data.get("fewestTurnsWin", 0))
    cumulative_turns.set(data.get("cumulativeTurns", 0))
    longest_win_streak.set(data.get("longestWinStreak", 0))
    longest_mastery_streak.set(data.get("longestMasteryStreak", 0))
    current_win_streak.set(data.get("currentWinStreak", 0))
    current_mastery_streak.set(data.get("currentMasteryStreak", 0))

    return jsonify({"status": "success"}), 200

# Endpoint to receive Item Metrics (POST)
@app.route('/items', methods=['POST'])
def receive_item_metrics():
    data = request.json
    app.logger.info("Received item metrics data: %s", data)
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    # Populate the items_data dictionary
    items_data.update(data)

    # Set Prometheus Gauges for weapon and armor details if fields are present
    weapon = data.get("weapon", {})
    armor = data.get("armor", {})

    if "damageMin" in weapon:
        item_weapon_damage_min.set(weapon["damageMin"])
    if "damageMax" in weapon:
        item_weapon_damage_max.set(weapon["damageMax"])
    if "defense" in armor:
        item_armor_defense.set(armor["defense"])

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

        # Record the creation time if this is a new monster
        if monster_id not in overall_monsters:
            monster["creation_time"] = time.time()
            monster_count.inc()
            app.logger.info("Added new monster to overall monsters: %s", monster)

        # Update or add the monster in `overall_monsters`
        overall_monsters[monster_id] = {**overall_monsters.get(monster_id, {}), **monster}

        # Ensure `current_game_monsters` has the latest data for active monsters
        if not monster.get("isDead", False):
            current_game_monsters[monster_id] = monster

    return jsonify({"status": "success", "received": monsters}), 200

# Endpoint to receive monster death notifications
@app.route('/monster/death', methods=['POST'])
def receive_monster_death():
    data = request.json
    monster_id = data.get("id")
    if not data or monster_id is None:
        return jsonify({"error": "No valid JSON payload received"}), 400

    app.logger.info("Received death notification for monster ID: %s", monster_id)

    if monster_id in overall_monsters:
        # Calculate lifespan and record the death time
        creation_time = overall_monsters[monster_id].get("creation_time")
        if creation_time:
            lifespan = time.time() - creation_time
            monster_lifespan_histogram.observe(lifespan)

        last_monster_death.set_to_current_time()

        # Update the monster's `currentHP` to 0 and mark it as dead in `overall_monsters`
        overall_monsters[monster_id]["hp"] = 0
        overall_monsters[monster_id]["isDead"] = True

        # Move to dead monsters but keep it in `overall_monsters`
        dead_monsters[monster_id] = overall_monsters[monster_id]
        monster_death_count.inc()

        # Remove from `current_game_monsters`
        current_game_monsters.pop(monster_id, None)

    return jsonify({"status": "success", "id": monster_id}), 200

# Endpoint to reset monster lists for a new game
@app.route('/monsters/reset', methods=['POST'])
def reset_current_game_monsters():
    current_game_monsters.clear()  # Clear the current game monsters list
    dead_monsters.clear()  # Clear the dead monsters list as well
    app.logger.info("Current game monster data and dead monsters data have been reset for a new game.")
    return jsonify({"status": "success"}), 200

# Other endpoint examples for serving player, game state, and monster data
@app.route('/player/data', methods=['GET'])
def player_data_endpoint():
    return jsonify(player_info_data)

@app.route('/gamestate/data', methods=['GET'])
def gamestate_data_endpoint():
    return jsonify(game_state_data)

# Endpoint to serve game stats data (GET)
@app.route('/gamestats/data', methods=['GET'])
def gamestats_data_endpoint():
    return jsonify(game_stats_data)

@app.route('/items/data', methods=['GET'])
def items_data_endpoint():
    return jsonify(items_data)

@app.route('/monsters/data', methods=['GET'])
def monsters_endpoint():
    return jsonify({"current_game_monsters": list(current_game_monsters.values())})

@app.route('/pack/data', methods=['GET'])
def pack_data_endpoint():
    return jsonify(player_pack_data)

@app.route('/overall_monsters', methods=['GET'])
def overall_monsters_endpoint():
    return jsonify({"overall_monsters": list(overall_monsters.values())})

@app.route('/dead_monsters', methods=['GET'])
def dead_monsters_endpoint():
    return jsonify({"dead_monsters": list(dead_monsters.values())})

@app.route('/monster/timestamps', methods=['GET'])
def monster_timestamps():
    return jsonify({
        "last_monster_created": last_monster_created._value.get(),
        "last_monster_death": last_monster_death._value.get()
    })

# HTML page endpoints
@app.route('/', methods=['GET'])
def show_tracker():
    return render_template('tracker.html')

@app.route('/player', methods=['GET'])
def show_player():
    return render_template('player.html')

@app.route('/game', methods=['GET'])
def show_game_info():
    return render_template('game.html')

@app.route('/monsters', methods=['GET'])
def show_monsters():
    return render_template('monsters.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
