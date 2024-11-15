# app/models/game.py
from pydantic import BaseModel
from typing import Optional
from app.models.gamestate import GameState
from app.models.gamestats import GameStats

class Game(BaseModel):
    game_state: GameState
    game_stats: GameStats

    def get_game_info(self):
        return {
            "game_state": self.game_state.dict(),  # Pydantic's `dict()` method
            "game_stats": self.game_stats.dict(),  # Pydantic's `dict()` method
        }
