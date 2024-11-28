"""
This module defines the `gamestate` blueprint for managing the game state and exposing 
related metrics.

It provides endpoints for receiving game state data, storing it in memory, and retrieving 
the current game state. Additionally, it defines Prometheus metrics for tracking various 
aspects of the game, such as reward rooms, monster spawn fuse, and total gold generated.

Returns:
    None: This module does not return values directly but provides Flask routes and 
    Prometheus metrics.
"""
from flask import Blueprint, request, jsonify
from app.models.gamestate import GameState
from prometheus_client import Gauge

bp = Blueprint('gamestate', __name__)

# Game State Metrics (Prometheus gauges for tracking in-game data)
game_reward_rooms = Gauge('brogue_reward_rooms_generated', 'Number of reward rooms generated')
game_has_ended = Gauge('brogue_game_has_ended', 'Indicates if the game has ended')
game_monster_spawn_fuse = Gauge('brogue_monster_spawn_fuse', 'Time until a monster spawns')
game_gold_generated = Gauge('brogue_gold_generated', 'Total amount of gold generated')
game_absolute_turn_number = Gauge('brogue_absolute_turn_number',
                                  'Total turns since the beginning of the game')
game_milliseconds = Gauge('brogue_milliseconds_since_launch', 'Milliseconds since game launch')
game_xpxp_this_turn = Gauge('brogue_xpxp_this_turn', 'Squares explored this turn')

# In-memory storage for the current game state
game_state_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_game_state():
    """
    Receives game state data and updates the in-memory storage.

    This route accepts a POST request with the game state data in JSON format.
    The data is validated using the `GameState` model, and the in-memory game state storage
    is updated. If the request is successful, a JSON response containing the status and received 
    data is returned.
    
    Args:
        None: This is a POST endpoint that expects JSON data.

    Returns:
        Response: A JSON response with the status of the request and the received game state data,
        or an error message.
    """
    data = request.json

    # Check if the data is missing
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Create a GameState object from the received data to validate and parse it
        game_state = GameState(**data)

        # Update the in-memory game_state_data with the latest received game state
        # Use the model's model_dump() method to get the validated data as a dictionary
        game_state_data.update(game_state.model_dump())

        # Return a success response along with the received game state data
        return jsonify({"status": "success", "received": game_state.model_dump()}), 200
    
    except (ValueError, TypeError, KeyError) as e:
        # If an error occurs during parsing or validation, return an error response
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'])
def get_game_state():
    """
    Returns the current game state data.

    This route handles GET requests to retrieve the current in-memory game state.
    If the game state data is not available, it returns a 404 error with an appropriate message.
    
    Args:
        None: This is a GET endpoint that returns the current game state data.

    Returns:
        Response: A JSON response containing the current game state data, or an error message if
        data is not available.
    """
    # Check if the game state data exists in memory
    if not game_state_data:
        return jsonify({"error": "Game state data not available"}), 404

    # Return the game state data as a JSON response
    return jsonify(game_state_data)
