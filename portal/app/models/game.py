"""
This module defines the Game class, which represents a game instance in the application.

It contains two key attributes:
- `game_state`: An instance of the `GameState` model, which represents the state of the game.
- `game_stats`: An instance of the `GameStats` model, which represents various statistics 
  for the game.

Returns:
    None: No return values for this module.
"""
from pydantic import BaseModel
from app.models.gamestate import GameState
from app.models.gamestats import GameStats

class Game(BaseModel):
    """
    A class representing a game instance, encapsulating both game state and statistics.

    Args:
        game_state (GameState): The current state of the game.
        game_stats (GameStats): The statistics of the game.

    Returns:
        Game: A new instance of the `Game` class.
    """
    game_state: GameState
    game_stats: GameStats

    def get_game_info(self):
        """
        Retrieves the information of the game, including both the state and stats.

        This method uses `model_dump()` to serialize the `game_state` and `game_stats` 
        attributes into dictionaries.

        Returns:
            dict: A dictionary containing the serialized `game_state` and `game_stats`.
        """
        return {
            "game_state": self.game_state.model_dump(),
            "game_stats": self.game_stats.model_dump(),
        }
