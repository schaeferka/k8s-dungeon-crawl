"""
This module defines the `Player` class, which represents the player character's data in the game.

The `Player` class encapsulates various attributes that describe the player's current state, 
including health, strength, game progression, and special abilities or bonuses. These attributes
are essential for tracking the player's status and actions within the game.

Returns:
    None: This module does not return any values.
"""
from pydantic import BaseModel

class Player(BaseModel):
    """
    Represents the player character in the game.

    The `Player` class tracks the player's core stats, such as health, strength, and game progress,
    along with various bonuses and status effects that impact gameplay. It provides a model for
    representing the player's state and can be used to manage the player's attributes 
    during the game.

    Attributes:
        gold (int): The amount of gold the player currently possesses.
        depth_level (int): The current depth level in the game.
        deepest_level (int): The deepest level the player has reached in the game.
        current_hp (int): The player's current health points.
        max_hp (int): The player's maximum health points.
        strength (int): The player's strength attribute, influencing physical combat.
        player_turn_number (int): The current turn number in the game for the player.
        xpxp_this_turn (int): The experience points gained during the current turn.
        stealth_range (int): The range within which the player can use stealth.
        disturbed (bool): Whether the player has been disturbed (e.g., alerted enemies).
        regen_per_turn (int): The amount of health regenerated per turn.
        weakness_amount (int): The player's current weakness status, which may affect combat.
        poison_amount (int): The amount of poison affecting the player, if any.
        clairvoyance (int): The player's clairvoyance level, influencing their ability to 
            sense the environment.
        stealth_bonus (int): The player's bonus to stealth actions, improving sneaking and evasion.
        regeneration_bonus (int): The player's bonus to health regeneration per turn.
        light_multiplier (int): The player's ability to see in the dark or in low-light 
            conditions.
        awareness_bonus (int): The player's bonus to awareness, affecting detection of 
            hidden threats.
        transference (int): The player's transference ability, influencing the transfer 
            of abilities or energy.
        wisdom_bonus (int): The player's wisdom, which can affect decision-making or 
            magical abilities.
        reaping (int): The player's reaping ability, influencing damage dealt to enemies 
            in certain situations.
    """
    gold: int
    depth_level: int
    deepest_level: int
    current_hp: int
    max_hp: int
    strength: int
    player_turn_number: int
    xpxp_this_turn: int
    stealth_range: int
    disturbed: bool
    regen_per_turn: int
    weakness_amount: int
    poison_amount: int
    clairvoyance: int
    stealth_bonus: int
    regeneration_bonus: int
    light_multiplier: int
    awareness_bonus: int
    transference: int
    wisdom_bonus: int
    reaping: int
