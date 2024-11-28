"""
This module defines a Flask blueprint for the game-related routes.

The `game` route fetches and combines the game state and game statistics and displays them
on the game information page (`game.html`). It ensures that both the game state and game stats
are available before rendering the page, and returns error messages if there are issues with 
fetching the data.

Returns:
    None: This module doesn't return values directly, but provides routes for the Flask app.
"""
from flask import Blueprint, render_template, jsonify
from app.routes.gamestate import get_game_state
from app.routes.gamestats import get_game_stats
from app.models.game import Game

bp = Blueprint('game', __name__)

@bp.route('/', methods=['GET'])
def game():
    """
    Displays the game information page, combining GameState and GameStats separately.

    This route fetches the current game state and game statistics, ensuring that both pieces
    of data are available. If the data is valid, it is passed to a Jinja template to render the
    `game.html` page. If any of the data is missing or an error occurs, an error message is 
    returned.

    Returns:
        Response: A rendered HTML page displaying the game state and game stats, or an error 
        message.
    """
    try:
        # Fetch the game state and game stats using the defined functions
        game_state = get_game_state()  # This should return a dictionary containing game state data
        game_stats = get_game_stats()  # This should return a dictionary containing game statistics

        # Ensure the data is valid
        if not game_state or not game_stats:
            # If game state or game stats are missing, return a 500 error with a message
            return jsonify({"error": "Missing game state or game stats data"}), 500

        # Pass game state and game stats information separately to the template for rendering
        return render_template('game.html', game_state=game_state, game_stats=game_stats)

    except (ConnectionError, TimeoutError, ValueError) as e:
        # If any error occurs during the process, log the error and return an internal server
        # error message
        print(f"Error loading game page: {e}")
        # Return a 500 Internal Server Error response with a generic error message
        return jsonify({"error": "Internal server error"}), 500
