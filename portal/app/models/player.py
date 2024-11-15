# app/models/player.py
from pydantic import BaseModel
from typing import Optional

class Player(BaseModel):
    """
    Defines the schema for Player data received from the game.
    """
    gold: int
    depthLevel: int
    deepestLevel: int
    currentHP: int
    maxHP: int
    strength: int
    playerTurnNumber: int
    xpxpThisTurn: int
    stealthRange: int
    disturbed: bool
    regenPerTurn: int
    weaknessAmount: int
    poisonAmount: int
    clairvoyance: int
    stealthBonus: int
    regenerationBonus: int
    lightMultiplier: int
    awarenessBonus: int
    transference: int
    wisdomBonus: int
    reaping: int