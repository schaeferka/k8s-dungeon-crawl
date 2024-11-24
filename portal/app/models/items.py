from pydantic import BaseModel
from typing import Optional, List

# Define a model for the "damage" object
class Damage(BaseModel):
    min: int
    max: int

# Define the base model for item properties
class Item(BaseModel):
    category: str
    kind: str
    quantity: int
    inscription: Optional[str] = "None"  
    damage: Optional[Damage] = None  
    armor: Optional[int] = 0 
    charges: Optional[int] = 0
    timesEnchanted: Optional[int] = 0
    strengthRequired: Optional[int] = 0
    inventoryLetter: Optional[str] = " " 

# Define the main model for the entire data structure
class ItemMetrics(BaseModel):
    weapon: Item
    armor: Item
    ringLeft: Item
    ringRight: Item

# Model for sending inventory items in a "pack"
class Pack(BaseModel):
    pack: List[Item]
