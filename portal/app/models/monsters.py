"""
This module defines the data models for the monsters in the K8s Dungeon Crawl game.

It includes the following classes:

1. Position: Represents the position of a monster in the game world, with x and y coordinates.
2. Monster: Represents a monster's attributes and behaviors, including health, attack speed, 
   defense, damage, and position.
   
The Monster class is initialized with various attributes and includes methods for converting the
instance to a dictionary and logging its initialization.

Additionally, this module integrates with Flask for logging, and the Monster class uses Pydantic
for data validation and serialization.

Module Functions:
- None
"""
from typing import Optional
from pydantic import BaseModel

# Defines the Position class for tracking coordinates of a Monster
class Position(BaseModel):
    """Represents the position of a monster in a 2D space.

    Attributes:
        x (int): The x-coordinate of the monster's position.
        y (int): The y-coordinate of the monster's position.
    """
    x: int
    y: int

# Defines the Monster class representing a monster's characteristics in the game
class Monster(BaseModel):
    """Represents a monster in the game with various attributes and methods.

    Attributes:
        id (int): A unique identifier for the monster.
        name (str): The name of the monster.
        type (str): The type/category of the monster.
        hp (int): The current health points of the monster.
        max_hp (int): The maximum health points of the monster.
        is_dead (bool): A flag indicating whether the monster is dead.
        depth (int): The dungeon depth where the monster resides.
        position (Position): The position of the monster in the game.
        attack_speed (int): The speed at which the monster can attack.
        movement_speed (int): The speed at which the monster can move.
        accuracy (int): The accuracy of the monster's attacks.
        defense (int): The defense rating of the monster.
        damage_min (int): The minimum damage the monster can deal.
        damage_max (int): The maximum damage the monster can deal.
        turns_between_regen (int): The number of turns between health regeneration.
        spawn_timestamp (Optional[int]): The timestamp when the monster spawned.
        death_timestamp (Optional[int]): The timestamp when the monster died (if applicable).
    """
    id: int
    name: str
    type: str
    hp: int
    max_hp: int
    is_dead: bool
    depth: int
    position: Position
    attack_speed: int
    movement_speed: int
    accuracy: int
    defense: int
    damage_min: int
    damage_max: int
    turns_between_regen: int
    spawn_timestamp: Optional[int] = None
    death_timestamp: Optional[int] = None

    class Config:
        """Configuration class for Pydantic's alias generation.

        This class configures how the field names are aliased, converting
        them to lowercase.
        """
        @staticmethod
        def alias_generator(s: str) -> str:
            """Generates a lowercase alias for the field names."""
            return s.lower()

    def to_dict(self):
        """Converts the Monster instance to a dictionary.

        Returns:
            dict: A dictionary representation of the Monster instance,
                  with field names in lowercase.
        """
        return self.model_dump(by_alias=True)
