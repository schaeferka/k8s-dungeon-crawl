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
        rewardRoomsGenerated (Optional[int]): The number of reward rooms generated. Defaults to 0.
        goldGenerated (Optional[int]): The amount of gold generated in the game. Defaults to 0.
        currentDepth (Optional[int]): The current depth level in the game. Defaults to 0.
        deepestLevel (Optional[int]): The deepest level reached in the game. Defaults to 0.
        gameInProgress (Optional[bool]): Indicates if the game is still in progress. Defaults to False.
        gameHasEnded (Optional[bool]): Indicates if the game has ended. Defaults to False.
        easyMode (Optional[bool]): Indicates if the game is in easy mode. Defaults to False.
        seed (Optional[int]): The seed value used for the game's random number generator. Defaults to 0.
        RNG (Optional[int]): The current random number generator value. Defaults to 0.
        absoluteTurnNumber (Optional[int]): The absolute turn number in the game. Defaults to 0.
        milliseconds (Optional[int]): Time in milliseconds since the game started. Defaults to 0.
        monsterSpawnFuse (Optional[int]): The fuse time for monster spawning. Defaults to 0.
        xpxpThisTurn (Optional[int]): The experience points gained this turn. Defaults to 0.
    """
    wizard: Optional[bool] = False
    rewardRoomsGenerated: Optional[int] = 0
    goldGenerated: Optional[int] = 0
    currentDepth: Optional[int] = 0
    deepestLevel: Optional[int] = 0
    gameInProgress: Optional[bool] = False
    gameHasEnded: Optional[bool] = False
    easyMode: Optional[bool] = False
    seed: Optional[int] = 0
    RNG: Optional[int] = 0
    absoluteTurnNumber: Optional[int] = 0
    milliseconds: Optional[int] = 0
    monsterSpawnFuse: Optional[int] = 0
    xpxpThisTurn: Optional[int] = 0
