from pydantic import BaseModel
from typing import Optional

class Position(BaseModel):
    """
    Model for the position of the monster.
    """
    x: int
    y: int


class Monster(BaseModel):
    """
    Model for a monster in the game.
    """
    id: int
    name: str
    type: str
    hp: int
    maxHP: int
    dead: bool
    depth: int
    position: Position 
    attackSpeed: int
    movementSpeed: int
    accuracy: int
    defense: int
    damageMin: int
    damageMax: int
    turnsBetweenRegen: int
    spawnTimestamp: Optional[int] = None
    deathTimestamp: Optional[int] = None

    class Config:
        # This will allow the model to work directly with the JSON data
        # and also allow 'snake_case' conversion
        alias_generator = lambda s: s.lower()

    def to_dict(self):
        """
        Convert the model to a dictionary for easy JSON serialization.
        """
        return self.model_dump(by_alias=True)
