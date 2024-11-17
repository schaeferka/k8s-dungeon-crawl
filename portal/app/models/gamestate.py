# app/models/gamestate.py
from pydantic import BaseModel
from typing import Optional

class GameState(BaseModel):
    """
    Defines the schema for GameState data received from the game.
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
