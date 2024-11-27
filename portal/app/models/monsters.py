"""
This module defines the models for `Position` and `Monster` in the game.

The `Position` model represents the coordinates of a monster in the game world, while
the `Monster` model captures various attributes of a monster, such as its health,
damage, and position, along with timestamps for spawn and death events.

Returns:
    None: This module does not return any values.
"""
from typing import Optional
from pydantic import BaseModel

class Position(BaseModel):
    """
    Represents the position of a monster in the game world.

    The `Position` model stores the x and y coordinates of the monster's location on the game map.

    Attributes:
        x (int): The x-coordinate of the monster's position.
        y (int): The y-coordinate of the monster's position.

    Args:
        x (int): The x-coordinate of the position.
        y (int): The y-coordinate of the position.
    """
    x: int
    y: int


class Monster(BaseModel):
    """
    Represents a monster in the game.

    The `Monster` model captures various characteristics of a monster, such as its
    unique identifier, type, health, position, attack speed, movement speed, damage range,
    and timestamps for its spawn and death. It also tracks whether the monster is alive.

    Attributes:
        id (int): The unique identifier for the monster.
        name (str): The name of the monster.
        type (str): The type of the monster (e.g., "fire", "water").
        hp (int): The current health of the monster.
        maxHP (int): The maximum health of the monster.
        dead (bool): Whether the monster is dead.
        depth (int): The depth level at which the monster exists.
        position (Position): The current position of the monster on the game map.
        attackSpeed (int): The speed of the monster's attacks.
        movementSpeed (int): The movement speed of the monster.
        accuracy (int): The accuracy of the monster's attacks.
        defense (int): The defense value of the monster.
        damageMin (int): The minimum damage the monster can inflict.
        damageMax (int): The maximum damage the monster can inflict.
        turnsBetweenRegen (int): The number of turns between health regeneration for the monster.
        spawnTimestamp (Optional[int]): The timestamp when the monster spawned. Defaults to None.
        deathTimestamp (Optional[int]): The timestamp when the monster died. Defaults to None.
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
        """
        Configuration for the Monster model.

        This configures the alias generator to convert attribute names to lowercase
        when serializing the model. For example, `Monster.id` will be serialized as `id`.

        Args:
            alias_generator (lambda): A lambda function that converts attribute names to lowercase.
        """
        alias_generator = lambda s: s.lower()

    def to_dict(self):
        """
        Convert the Monster model to a dictionary for easy JSON serialization.

        This method serializes the model's attributes, including aliases, into a dictionary format
        which can be easily converted into a JSON response.

        Returns:
            dict: A dictionary representation of the Monster model.
        """
        return self.model_dump(by_alias=True)
