from flask import Blueprint, request, jsonify, render_template, current_app
from app.models.player import Player
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST

bp = Blueprint('player', __name__)

# In-memory storage
player_data = {}

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


@bp.route('/', methods=['GET'], strict_slashes=False)
def player():
    """
    Displays the player page.
    """
    return render_template('player.html')

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_player():
    """
    Receives player data and updates the in-memory storage.
    """
    data = request.json  # Expecting JSON format

    # Log the received data to verify the format
    # current_app.logger.info("Received player data: %s", data)

    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        player = Player(**data)  # Assuming Player is the model you're using
        player_data.update(data)
        # current_app.logger.info("Successfully processed player data: %s", data)
        return jsonify({"status": "success", "received": player.dict()}), 200
    except Exception as e:
        # Log the error if there's a problem with the player data
        current_app.logger.error(f"Error processing player data: {str(e)}")
        return jsonify({"error": str(e)}), 400


@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_player():
    """
    Returns the current player data.
    """
    return jsonify(player_data)


