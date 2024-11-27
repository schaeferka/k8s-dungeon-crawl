"""
This module defines the `GameService` class, which is responsible for managing and updating
the game state and game statistics. It provides functionality to update, retrieve, and reset 
the game state and stats, and integrates with Prometheus to expose game-related metrics.

The `GameService` class offers the following features:
- Updating the game state and game statistics with new data.
- Resetting the game state and statistics for a new session.
- Retrieving the current game state and statistics.
- Integrating with Prometheus to track metrics such as reward rooms generated, gold collected,
  game progression, and more.

Prometheus metrics tracked by this module include:
- `brogue_reward_rooms_generated`: Number of reward rooms generated.
- `brogue_monster_spawn_fuse`: Time until a monster spawns.
- `brogue_gold_generated`: Total gold generated.
- `brogue_absolute_turn_number`: Total turns since the game started.
- `brogue_milliseconds_since_launch`: Time elapsed since the game launched.
- `brogue_game_has_ended`: Whether the game has ended.
- `brogue_games_played`: Total number of games played.
- `brogue_games_won`: Total number of games won.
- `brogue_game_deepest_level`: Deepest level reached in any game.
- `brogue_cumulative_gold`: Total gold collected across all games.

This module provides an interface for interacting with the game state, game statistics, and 
tracking key gameplay metrics.
"""
from prometheus_client import Gauge, Counter
from app.models.gamestate import GameState
from app.models.gamestats import GameStats

# Prometheus metrics for game state
reward_rooms_generated = Gauge(
    'brogue_reward_rooms_generated', 
    'Number of reward rooms generated'
)
monster_spawn_fuse = Gauge(
    'brogue_monster_spawn_fuse', 
    'Time until a monster spawns'
)
gold_generated = Gauge(
    'brogue_gold_generated', 
    'Total amount of gold generated'
)
absolute_turn_number = Gauge(
    'brogue_absolute_turn_number', 
    'Total turns since the beginning of the game'
)
milliseconds_since_launch = Gauge(
    'brogue_milliseconds_since_launch', 
    'Milliseconds since game launch'
)
game_has_ended = Gauge(
    'brogue_game_has_ended', 
    'Indicates if the game has ended'
)

# Prometheus metrics for game stats
games_played = Counter(
    'brogue_games_played', 
    'Total games played'
)
games_won = Counter(
    'brogue_games_won', 
    'Total games won'
)
deepest_level_reached = Gauge(
    'brogue_game_deepest_level', 
    'Deepest level reached in any game'
)
cumulative_gold_collected = Gauge(
    'brogue_cumulative_gold', 
    'Total gold collected across all games'
)

game_state_data = {}
game_stats_data = {}

class GameService:
    """
    Service class for handling game state and game statistics updates, retrieval, and reset.
    This class interacts with Prometheus metrics to track game state and statistics.
    """

    @staticmethod
    def update_game_state(data: dict):
        """
        Update the game state with the provided data.

        This method updates the game state in memory and reflects the changes in Prometheus metrics.
        It checks if each value is not None before updating the corresponding metric.

        Args:
            data (dict): A dictionary containing the new game state data.

        Returns:
            dict: A dictionary containing the status of the update and the updated game state.
        """
        try:
            game_state = GameState(**data)
            game_state_data.update(game_state.dict())

            # Update Prometheus metrics for game state
            if game_state.rewardRoomsGenerated is not None:
                reward_rooms_generated.set(game_state.rewardRoomsGenerated)
            if game_state.monsterSpawnFuse is not None:
                monster_spawn_fuse.set(game_state.monsterSpawnFuse)
            if game_state.goldGenerated is not None:
                gold_generated.set(game_state.goldGenerated)
            if game_state.absoluteTurnNumber is not None:
                absolute_turn_number.set(game_state.absoluteTurnNumber)
            if game_state.milliseconds is not None:
                milliseconds_since_launch.set(game_state.milliseconds)
            if game_state.gameHasEnded is not None:
                game_has_ended.set(1 if game_state.gameHasEnded else 0)

            return {"status": "success", "updated": game_state.dict()}
        except (TypeError, ValueError, KeyError) as e:
            return {"status": "error", "message": str(e)}

    @staticmethod
    def update_game_stats(data: dict):
        """
        Update the game statistics with the provided data.

        This method updates the game statistics in memory and updates the corresponding Prometheus 
        metrics. It checks if the stats are valid and then updates the counters and gauges for 
        games played, games won, deepest level reached, and cumulative gold collected.

        Args:
            data (dict): A dictionary containing the new game statistics data.

        Returns:
            dict: A dictionary containing the status of the update and the updated game stats.
        """
        try:
            game_stats = GameStats(**data)
            game_stats_data.update(game_stats.dict())

            # Update Prometheus metrics for game stats
            if game_stats.games_played:
                games_played.inc()
            if game_stats.games_won:
                games_won.inc()
            if game_stats.deepestLevel is not None:
                deepest_level_reached.set(game_stats.deepestLevel)
            if game_stats.cumulativeGold is not None:
                cumulative_gold_collected.set(game_stats.cumulativeGold)

            return {"status": "success", "updated": game_stats.dict()}
        except (TypeError, ValueError, KeyError) as e:
            return {"status": "error", "message": str(e)}

    @staticmethod
    def reset_game_state():
        """
        Reset the game state for a new game.

        This method clears the in-memory storage for the game state, preparing the system for a
        fresh game session.

        Returns:
            dict: A dictionary containing the status of the reset operation.
        """
        game_state_data.clear()
        return {"status": "success", "message": "Game state reset successfully."}

    @staticmethod
    def reset_game_stats():
        """
        Reset the game statistics.

        This method clears the in-memory storage for game statistics, ensuring the metrics start
        fresh for a new game session.

        Returns:
            dict: A dictionary containing the status of the reset operation.
        """
        game_stats_data.clear()
        return {"status": "success", "message": "Game stats reset successfully."}

    @staticmethod
    def get_game_state():
        """
        Retrieve the current game state.

        This method returns the current game state stored in memory.

        Returns:
            dict: The current game state data.
        """
        return game_state_data

    @staticmethod
    def get_game_stats():
        """
        Retrieve the current game statistics.

        This method returns the current game statistics stored in memory.

        Returns:
            dict: The current game statistics data.
        """
        return game_stats_data
