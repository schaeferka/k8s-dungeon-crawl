from flask import Blueprint, request, jsonify
from app.models.gamestate import GameState
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST


bp = Blueprint('gamestate', __name__)

# Game State Metrics
game_reward_rooms = Gauge('brogue_reward_rooms_generated', 'Number of reward rooms generated')
game_has_ended = Gauge('brogue_game_has_ended', 'Indicates if the game has ended')
game_monster_spawn_fuse = Gauge('brogue_monster_spawn_fuse', 'Time until a monster spawns')
game_gold_generated = Gauge('brogue_gold_generated', 'Total amount of gold generated')
game_absolute_turn_number = Gauge('brogue_absolute_turn_number', 'Total turns since the beginning of the game')
game_milliseconds = Gauge('brogue_milliseconds_since_launch', 'Milliseconds since game launch')
game_xpxp_this_turn = Gauge('brogue_xpxp_this_turn', 'Squares explored this turn')

# In-memory storage for the current game state
game_state_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_game_state():
    """
    Receives game state data and updates the in-memory storage.
    """
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Create a GameState object from the received data and update game_state_data
        game_state = GameState(**data)  # Validate and parse the incoming JSON using the GameState model
        
        # Update the in-memory game_state_data with the latest game state
        game_state_data.update(data)

        # Return the game state back in the response
        return jsonify({"status": "success", "received": game_state.dict()}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'])
def get_game_state():
    """
    Returns the current game state data.
    """
    if not game_state_data:
        return jsonify({"error": "Game state data not available"}), 404

    return game_state_data
