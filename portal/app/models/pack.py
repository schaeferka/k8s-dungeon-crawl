"""
This module defines the models for `PackItem` and `Pack`.

The `PackItem` model represents an individual item in a player's inventory pack, with attributes such as category, name, and quantity.
The `Pack` model represents a collection of items in the player's inventory, encapsulating a list of `PackItem` instances.

Returns:
    None: This module does not return any values.
"""
from typing import List, Optional
from pydantic import BaseModel

class PackItem(BaseModel):
    """
    Represents an individual item in a player's inventory pack.

    The `PackItem` model stores the essential details of an item, including its category, name, quantity, 
    and a letter representing its position in the inventory.

    Attributes:
        category (str): The category of the item (e.g., "weapon", "armor").
        name (str): The name of the item (e.g., "sword", "shield").
        quantity (int): The quantity of the item in the player's inventory.
        inventory_letter (str): The letter representing the item in the player's inventory.

    Args:
        category (str): The category of the item (e.g., "weapon", "armor").
        name (str): The name of the item (e.g., "sword", "shield").
        quantity (int): The quantity of the item in the player's inventory.
        inventory_letter (str): The letter used to identify the item in the inventory.
    """
    category: str
    name: str
    quantity: int
    inventory_letter: str

class Pack(BaseModel):
    """
    Represents a player's inventory pack, containing a list of items.

    The `Pack` model holds the player's inventory, which is a collection of `PackItem` instances.
    It can represent the entire inventory of the player, with each item having specific attributes like name,
    category, quantity, and inventory letter.

    Attributes:
        pack (Optional[List[PackItem]]): A list of `PackItem` instances representing the items in the player's inventory. Defaults to None.

    Args:
        pack (Optional[List[PackItem]]): A list of `PackItem` instances. If no items are in the pack, this will be None.
    """
    pack: Optional[List[PackItem]] = None
