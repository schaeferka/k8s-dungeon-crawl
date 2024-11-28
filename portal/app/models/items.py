"""
This module defines classes that represent various game items and their attributes,
including damage, item metrics, and inventory packs. It uses the Pydantic `BaseModel`
to enforce data validation and serialization.

Classes:
    Damage: Represents the damage range of a weapon.
    Item: Represents an item in the game, including properties like category, kind, and damage.
    EquippedItems: Represents the player's equipped items, including weapon, armor, and rings.
    Pack: Represents an inventory pack that contains a list of items.

Returns:
    None: No return values for this module.
"""
from typing import Optional, List
from pydantic import BaseModel
from flask import current_app

class Damage(BaseModel):
    """
    Represents the damage range of a weapon.

    This class tracks the minimum and maximum damage a weapon can inflict during combat.

    Attributes:
        min (int): The minimum damage the weapon can inflict.
        max (int): The maximum damage the weapon can inflict.

    Args:
        min (int): The minimum damage value.
        max (int): The maximum damage value.
    """
    min: int
    max: int

class Item(BaseModel):
    """
    Represents an item in the game.

    An item can be a weapon, armor, or other equipment. It includes properties like category,
    kind, damage (if applicable), enchantments, and more.

    Attributes:
        category (str): The category of the item (e.g., "weapon", "armor").
        kind (str): The type or kind of the item (e.g., "sword", "helmet").
        quantity (int): The quantity of the item in the player's inventory.
        inscription (Optional[str]): An inscription on the item, if any. Defaults to "None".
        damage (Optional[Damage]): The damage range of the item, if it's a weapon. Defaults to None.
        armor (Optional[int]): The armor value of the item, if applicable. Defaults to 0.
        charges (Optional[int]): The number of charges left for items like scrolls or potions.
        times_enchanted (Optional[int]): The number of times the item has been enchanted.
        strength_required (Optional[int]): The strength required to equip the item.
        inventory_letter (Optional[str]): The letter representing the item in the player's inventory.
        origin_depth (Optional[int]): The depth at which the item was found in the game world.
        enchant1 (Optional[str]): The first enchantment applied to the item. Defaults to "None".
        enchant2 (Optional[str]): The second enchantment applied to the item. Defaults to "None".
        description (Optional[str]): A brief description of the item. Defaults to "No description 
            available".

    Args:
        category (str): The category of the item (e.g., "weapon", "armor").
        kind (str): The type or kind of the item (e.g., "sword", "helmet").
        quantity (int): The quantity of the item in the player's inventory.
    """
    category: Optional[str] = "None"
    kind: Optional[str] = "None"
    quantity: Optional[int] = 0
    inscription: Optional[str] = "None"
    damage: Optional[Damage] = None
    armor: Optional[int] = 0
    charges: Optional[int] = 0
    times_enchanted: Optional[int] = 0
    strength_required: Optional[int] = 0
    inventory_letter: Optional[str] = " "
    origin_depth: Optional[int] = 0
    enchant1: Optional[str] = "None"
    enchant2: Optional[str] = "None"
    description: Optional[str] = "No description available"

class EquippedItems(BaseModel):
    """
    Represents the player's equipped items, including their weapon, armor, and rings.

    This class tracks the equipment currently worn or used by the player, including their weapon,
    armor, and rings on the left and right hands.

    Attributes:
        weapon (Item): The player's equipped weapon.
        armor (Item): The player's equipped armor.
        left_ring (Item): The player's equipped ring in the left hand.
        right_ring (Item): The player's equipped ring in the right hand.

    Args:
        weapon (Item): The item representing the player's weapon.
        armor (Item): The item representing the player's armor.
        left_ring (Item): The item representing the player's left-hand ring.
        right_ring (Item): The item representing the player's right-hand ring.
    """
    weapon: Item
    armor: Item
    left_ring: Item
    right_ring: Item

class Pack(BaseModel):
    """
    Represents an inventory pack containing a list of items.

    This class holds the items in the player's pack, which could include weapons, armor,
    consumables, and other items that are not currently equipped.

    Attributes:
        pack (List[Item]): A list of items in the player's inventory pack.

    Args:
        pack (List[Item]): A list of items contained in the pack.
    """
    pack: List[Item]

class Items(BaseModel):
    """
    Represents the collection of items in the game.

    This class encapsulates the player's equipped items, inventory pack, and other item-related
    data structures.

    Attributes:
        equipped_items (EquippedItems): The player's equipped items.
        pack (Pack): The player's inventory pack.

    Args:
        equipped_items (EquippedItems): The player's equipped items.
        pack (Pack): The player's inventory pack.
    """
    equipped_items: Optional[EquippedItems] = None
    pack_items: Optional[Pack] = None

