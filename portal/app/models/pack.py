# app/models/pack.py
from pydantic import BaseModel
from typing import List, Optional

class PackItem(BaseModel):
    category: str
    name: str
    quantity: int
    inventory_letter: str

class Pack(BaseModel):
    """
    Defines the schema for Pack data received from the game.
    """
    pack: Optional[List[PackItem]] = None
