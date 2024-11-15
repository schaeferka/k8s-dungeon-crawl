from prometheus_client import Gauge, Counter
from app.models.gamestate import GameState
from app.models.gamestats import GameStats

# Prometheus metrics for game state
reward_rooms_generated = Gauge('brogue_reward_rooms_generated', 'Number of reward rooms generated')
monster_spawn_fuse = Gauge('brogue_monster_spawn_fuse', 'Time until a monster spawns')
gold_generated = Gauge('brogue_gold_generated', 'Total amount of gold generated')
absolute_turn_number = Gauge('brogue_absolute_turn_number', 'Total turns since the beginning of the game')
milliseconds_since_launch = Gauge('brogue_milliseconds_since_launch', 'Milliseconds since game launch')
game_has_ended = Gauge('brogue_game_has_ended', 'Indicates if the game has ended')

# Prometheus metrics for game stats
games_played = Counter('brogue_games_played', 'Total games played')
games_won = Counter('brogue_games_won', 'Total games won')
deepest_level_reached = Gauge('brogue_game_deepest_level', 'Deepest level reached in any game')
cumulative_gold_collected = Gauge('brogue_cumulative_gold', 'Total gold collected across all games')

# In-memory storage for game state and stats
game_state_data = {}
game_stats_data = {}

class GameService:
    @staticmethod
    def update_game_state(data: dict):
        """
        Update the game state with the provided data.
        """
        try:
            game_state = GameState(**data)
            game_state_data.update(game_state.dict())

            # Update Prometheus metrics
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
        except Exception as e:
            return {"status": "error", "message": str(e)}

    @staticmethod
    def update_game_stats(data: dict):
        """
        Update the game stats with the provided data.
        """
        try:
            game_stats = GameStats(**data)
            game_stats_data.update(game_stats.dict())

            # Update Prometheus metrics
            games_played.inc() if game_stats.games else None
            games_won.inc() if game_stats.won else None
            if game_stats.deepestLevel is not None:
                deepest_level_reached.set(game_stats.deepestLevel)
            if game_stats.cumulativeGold is not None:
                cumulative_gold_collected.set(game_stats.cumulativeGold)

            return {"status": "success", "updated": game_stats.dict()}
        except Exception as e:
            return {"status": "error", "message": str(e)}

    @staticmethod
    def reset_game_state():
        """
        Reset the game state for a new game.
        """
        global game_state_data
        game_state_data.clear()
        return {"status": "success", "message": "Game state reset successfully."}

    @staticmethod
    def reset_game_stats():
        """
        Reset the game stats.
        """
        global game_stats_data
        game_stats_data.clear()
        return {"status": "success", "message": "Game stats reset successfully."}

    @staticmethod
    def get_game_state():
        """
        Retrieve the current game state.
        """
        return game_state_data

    @staticmethod
    def get_game_stats():
        """
        Retrieve the current game stats.
        """
        return game_stats_data
