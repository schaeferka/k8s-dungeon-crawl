# app/models/items.py
from pydantic import BaseModel
from typing import Optional

class Weapon(BaseModel):
    damage_min: Optional[int] = None
    damage_max: Optional[int] = None

class Armor(BaseModel):
    defense: Optional[int] = None

class Items(BaseModel):
    """
    Defines the schema for Items data received from the game.
    """
    weapon: Optional[Weapon] = None
    armor: Optional[Armor] = None
