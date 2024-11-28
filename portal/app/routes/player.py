"""
This module defines the API endpoints for managing player data in the K8s Dungeon Crawl game.

It includes the following functionalities:

1. Displaying the player data on a dedicated player page.
2. Retrieving specific player metrics such as health points, gold, depth, strength, etc.
3. Receiving and updating player data via a POST request, validating the input using the Player model.
4. Providing the current state of the player data, including various in-game statistics.

Endpoints:
- /hp: Returns the current player hit points (hp).
- /gold: Returns the amount of gold collected by the player.
- /depth: Returns the current depth level of the player.
- /deepest-level: Returns the deepest level the player has reached.
- /max-hp: Returns the maximum hit points (hp) of the player.
- /strength: Returns the current strength of the player.
- /: Displays the player page.
- /update: Receives player data via a POST request and updates the in-memory storage.
- /data: Returns the current player data as a JSON response.

The player data is stored in an in-memory dictionary and validated using the Player model.
"""
from flask import Blueprint, request, jsonify, render_template, current_app
from app.models.player import Player
from prometheus_client import Gauge

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
player_stealth_range = Gauge('brogue_stealth_range',
                             'Distance from which monsters will notice the player')
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


@bp.route('/hp', methods=['GET'], strict_slashes=False)
def get_hp():
    """
    Returns the current player hp.

    Retrieves the current hit points (hp) of the player from the in-memory storage
    and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the player's current hit points.
    """
    hp = player_data['current_hp']
    return jsonify({"hp": hp})


@bp.route('/gold', methods=['GET'], strict_slashes=False)
def get_gold():
    """
    Returns the current player gold.

    Retrieves the amount of gold the player has collected from the in-memory storage
    and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the player's gold amount.
    """
    gold = player_data['gold']
    return jsonify({"gold": gold})


@bp.route('/depth', methods=['GET'], strict_slashes=False)
def get_depth():
    """
    Returns the current player depth.

    Retrieves the current depth level of the player from the in-memory storage
    and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the player's current depth level.
    """
    depth = player_data['depth_level']
    return jsonify({"current_depth": depth})


@bp.route('/deepest-level', methods=['GET'], strict_slashes=False)
def get_deepest_level():
    """
    Returns the current player deepest level.

    Retrieves the deepest level reached by the player from the in-memory storage
    and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the player's deepest level reached.
    """
    depth = player_data['deepest_level']
    return jsonify({"deepest_level": depth})


@bp.route('/max-hp', methods=['GET'], strict_slashes=False)
def get_max_hp():
    """
    Returns the current player max hp.

    Retrieves the maximum hit points (hp) of the player from the in-memory storage
    and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the player's maximum hit points.
    """
    hp = player_data['max_hp']
    return jsonify({"max_hp": hp})


@bp.route('/strength', methods=['GET'], strict_slashes=False)
def get_strength():
    """
    Returns the current player strength.

    Retrieves the strength of the player from the in-memory storage
    and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the player's strength.
    """
    strength = player_data['strength']
    return jsonify({"strength": strength})


@bp.route('/', methods=['GET'], strict_slashes=False)
def player():
    """
    Displays the player page.

    Renders the player page as an HTML template, displaying the player's current stats.

    Returns:
        Response: The rendered HTML page showing the player's data.
    """
    return render_template('player.html')


@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_player():
    """
    Receives player data and updates the in-memory storage.

    This function validates the incoming JSON data using the Player model and updates the
    in-memory storage with the received player data. If any validation errors occur, a
    400 response is returned.

    Returns:
        Response: A JSON response indicating the status of the update operation.
    """
    data = request.json  # Expecting JSON format

    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate the player data using the Player model
        new_player = Player(**data)
        player_data.update(data)
        return jsonify({"status": "success", "received": new_player.model_dump()}), 200
    except ValueError as e:
        current_app.logger.error(f"Error processing player data: {str(e)}")
        return jsonify({"error": str(e)}), 400


@bp.route('/data', methods=['GET'], strict_slashes=False)
def get_player():
    """
    Returns the current player data.

    Retrieves the current player data from the in-memory storage and returns it as a JSON response.

    Returns:
        Response: A JSON response containing the current player data.
    """
    return jsonify(player_data)
