from flask import Blueprint, request, jsonify
from app.models.gamestats import GameStats
from prometheus_client import Gauge, Counter, Histogram, generate_latest, CONTENT_TYPE_LATEST


bp = Blueprint('gamestats', __name__)

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

# In-memory storage for the current game stats
game_stats_data = {}

@bp.route('/update', methods=['POST'], strict_slashes=False)
def receive_game_stats():
    """
    Receives game stats data and updates the in-memory storage.
    """
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    try:
        # Validate and parse the incoming JSON using the GameStats model
        game_stats = GameStats(**data)  # Creates a GameStats instance and validates the data
        
        # Update the in-memory game_stats_data with the latest game stats
        game_stats_data.update(data)

        # Return the game stats back in the response
        return jsonify({"status": "success", "received": game_stats.dict()}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 400

@bp.route('/data', methods=['GET'])
def get_game_stats():
    """
    Returns the current game stats data.
    """
    if not game_stats_data:
        return jsonify({"error": "Game stats data not available"}), 404

    return game_stats_data 
