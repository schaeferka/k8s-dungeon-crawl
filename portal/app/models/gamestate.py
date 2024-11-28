"""
This module defines the `GameState` class, which represents the state of the game.
It uses Pydantic's `BaseModel` to enforce data validation and provide serialization
for the game state data.

The class contains attributes that track various aspects of the game's state,
such as the wizard status, game progression, rewards, and game-related time metrics.

Returns:
    None: No return values for this module.
"""
from typing import Optional
from pydantic import BaseModel

class GameState(BaseModel):
    """
    Represents the state of the game at a given point in time.

    This class defines the schema for various attributes that describe the current
    status of the game, including game progression, the player's wizard status,
    generated rewards, and time-related information. All attributes are optional
    and have default values indicating the initial state of the game.

    Attributes:
        wizard (Optional[bool]): Indicates if the player is a wizard. Defaults to False.
        reward_rooms_generated (Optional[int]): The number of reward rooms generated. Defaults to 0.
        gold_generated (Optional[int]): The amount of gold generated in the game. Defaults to 0.
        current_depth (Optional[int]): The current depth level in the game. Defaults to 0.
        deepest_level (Optional[int]): The deepest level reached in the game. Defaults to 0.
        game_in_progress (Optional[bool]): Indicates if the game is still in progress. Defaults to False.
        game_has_ended (Optional[bool]): Indicates if the game has ended. Defaults to False.
        easy_mode (Optional[bool]): Indicates if the game is in easy mode. Defaults to False.
        seed (Optional[int]): The seed value used for the game's random number generator. Defaults to 0.
        rng (Optional[int]): The current random number generator value. Defaults to 0.
        absolute_turn_number (Optional[int]): The absolute turn number in the game. Defaults to 0.
        milliseconds (Optional[int]): Time in milliseconds since the game started. Defaults to 0.
        monster_spawn_fuse (Optional[int]): The fuse time for monster spawning. Defaults to 0.
        xpxp_this_turn (Optional[int]): The experience points gained this turn. Defaults to 0.
    """
    wizard: Optional[bool] = False
    reward_rooms_generated: Optional[int] = 0
    gold_generated: Optional[int] = 0
    current_depth: Optional[int] = 0
    deepest_level: Optional[int] = 0
    game_in_progress: Optional[bool] = False
    game_has_ended: Optional[bool] = False
    easy_mode: Optional[bool] = False
    seed: Optional[int] = 0
    rng: Optional[int] = 0
    absolute_turn_number: Optional[int] = 0
    milliseconds: Optional[int] = 0
    monster_spawn_fuse: Optional[int] = 0
    xpxp_this_turn: Optional[int] = 0
