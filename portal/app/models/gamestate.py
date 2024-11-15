# app/models/gamestate.py
from pydantic import BaseModel
from typing import Optional

class GameState(BaseModel):
    """
    Defines the schema for GameState data received from the game.
    """
    reward_rooms_generated: Optional[int] = None
    monster_spawn_fuse: Optional[int] = None
    gold_generated: Optional[int] = None
    absolute_turn_number: Optional[int] = None
    milliseconds_since_launch: Optional[int] = None
    xpxp_this_turn: Optional[int] = None
    game_has_ended: Optional[bool] = None
