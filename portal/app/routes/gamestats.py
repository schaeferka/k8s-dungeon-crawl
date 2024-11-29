"""
This module defines the `gamestats` blueprint for managing game statistics and exposing
related metrics.

It provides endpoints for receiving game stats data, storing it in memory, and retrieving
the current game stats. Additionally, it defines Prometheus metrics for tracking various
aspects of the game, such as the number of games played, the highest score, win streaks,
and other cumulative game stats.

Returns:
    None: This module does not return any values directly but provides Flask routes and 
    Prometheus metrics.
"""
from flask import Blueprint, request, jsonify
from app.models.gamestats import GameStats
from prometheus_client import Gauge

bp = Blueprint('gamestats', __name__)

# Prometheus Gauges for Game Stats (These will track various game metrics)
games_played = Gauge('brogue_games_played', 'Total games played')
games_escaped = Gauge('brogue_games_escaped', 'Total games escaped')
games_mastered = Gauge('brogue_games_mastered', 'Total games mastered')
games_won = Gauge('brogue_games_won', 'Total games won')
win_rate = Gauge('brogue_win_rate', 'Win rate as a percentage')
deepest_level = Gauge('brogue_game_deepest_level', 'Deepest level reached')
cumulative_levels = Gauge('brogue_cumulative_levels',
                          'Total cumulative levels reached across all games')
highest_score = Gauge('brogue_highest_score', 'Highest score achieved')
cumulative_score = Gauge('brogue_cumulative_score', 'Cumulative score across all games')
most_gold = Gauge('brogue_most_gold', 'Most gold collected in a single game')
cumulative_gold = Gauge('brogue_cumulative_gold', 'Cumulative gold collected across all games')
most_lumenstones = Gauge('brogue_most_lumenstones', 'Most lumenstones collected in a single game')
cumulative_lumenstones = Gauge('brogue_cumulative_lumenstones',
                               'Cumulative lumenstones collected across all games')
fewest_turns_win = Gauge('brogue_fewest_turns_win', 'Fewest turns taken to win a game')
cumulative_turns = Gauge('brogue_cumulative_turns', 'Cumulative turns played across all games')
longest_win_streak = Gauge('brogue_longest_win_streak', 'Longest win streak')
longest_mastery_streak = Gauge('brogue_longest_mastery_streak', 'Longest mastery streak')
current_win_streak = Gauge('brogue_current_win_streak', 'Current win streak')
current_mastery_streak = Gauge('brogue_current_mastery_streak', 'Current mastery streak')

# In-memory storage for the current game stats
game_stats_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_game_stats():
    """
    Receives game stats data and updates the in-memory storage.

    This route accepts a POST request with game statistics data in JSON format. It validates
    and parses the incoming data using the `GameStats` model. If the data is valid, it updates 
    the in-memory storage of game stats and returns a success response.

    If the request does not contain valid JSON or the data cannot be parsed, an error response
    is returned.

    Returns:
        Response: A JSON response with the status of the request and the received game stats 
        data or an error message.
    """
    data = request.json

    # Check if the request contains data
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate and parse the incoming JSON using the GameStats model
        game_stats = GameStats(**data)  # Creates a GameStats instance and validates the data

        # Update the in-memory game_stats_data with the latest received game stats
        # Use model_dump to store the validated fields from the GameStats model
        game_stats_data.update(game_stats.model_dump())

        # Return the updated game stats in the response
        return jsonify({"status": "success", "received": game_stats.model_dump()}), 200
    except ValueError as e:
        # Return an error response if validation fails or if there's any issue processing the data
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'])
def get_game_stats():
    """
    Returns the current game stats data.

    This route handles GET requests to retrieve the current in-memory game stats. If the game 
    stats are available, it returns them in the response. If not, it returns a 404 error message 
    indicating that the game stats are not available.

    Returns:
        Response: A JSON response containing the current game stats or an error message if 
        no stats are available.
    """
    # Check if game stats data exists in memory
    if not game_stats_data:
        return jsonify({"error": "Game stats data not available"}), 404

    # Return the game stats data as a JSON response
    return jsonify(game_stats_data)
