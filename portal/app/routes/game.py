# In app/routes/game.py
from flask import Blueprint, render_template, jsonify
from app.routes.gamestate import get_game_state
from app.routes.gamestats import get_game_stats

bp = Blueprint('game', __name__)

@bp.route('/', methods=['GET'])
def game():
    """
    Displays the game information page, combining GameState and GameStats separately.
    """
    try:
        # Fetch the game state and game stats using the defined functions
        game_state = get_game_state()  # This should return a dictionary
        game_stats = get_game_stats()  # This should return a dictionary

        # Ensure the data is valid
        if not game_state or not game_stats:
            return jsonify({"error": "Missing game state or game stats data"}), 500

        # Pass game state and game stats information separately to the template
        return render_template('game.html', game_state=game_state, game_stats=game_stats)

    except Exception as e:
        # If any error occurs, log and return an internal server error message
        print(f"Error loading game page: {e}")
        return jsonify({"error": "Internal server error"}), 500
