"""
This module defines the `GameStats` class, which represents the game statistics
for a player, tracking various metrics about gameplay history.

The class uses Pydantic's `BaseModel` for data validation and serialization.
It contains attributes that track statistics such as games played, games won,
scores, and performance metrics across different game sessions.

Returns:
    None: No return values for this module.
"""
from typing import Optional
from pydantic import BaseModel

class GameStats(BaseModel):
    """
    Represents the statistics of a player's gameplay, including various
    performance metrics across all game sessions.

    This class defines the schema for tracking the number of games played,
    won, escaped, and mastered, as well as detailed statistics like win rate,
    scores, gold, and streaks.

    Attributes:
        games_played (int): The total number of games played. Defaults to 0.
        games_escaped (int): The number of games the player has escaped. Defaults to 0.
        games_mastered (int): The number of games the player has mastered. Defaults to 0.
        games_won (int): The number of games won by the player. Defaults to 0.
        win_rate (Optional[float]): The player's win rate, calculated as 
            games_won / games_played. Defaults to None.
        deepest_level (Optional[int]): The deepest level reached in any game. Defaults to None.
        cumulative_levels (Optional[int]): The total number of levels across all games.
            Defaults to None.
        highest_score (Optional[int]): The highest score achieved in a game. Defaults to None.
        cumulative_score (Optional[int]): The cumulative score across all games. Defaults to None.
        most_gold (Optional[int]): The most gold collected in a single game. Defaults to None.
        cumulative_gold (Optional[int]): The cumulative gold collected across all games.
            Defaults to None.
        most_lumenstones (Optional[int]): The most lumenstones collected in a single game.
            Defaults to None.
        cumulative_lumenstones (Optional[int]): The cumulative number of lumenstones collected 
            across all games. Defaults to None.
        fewest_turns_win (Optional[int]): The fewest number of turns taken to win a game.
            Defaults to None.
        cumulative_turns (Optional[int]): The total number of turns taken across all games.
            Defaults to None.
        longest_win_streak (Optional[int]): The longest consecutive win streak. Defaults to None.
        longest_mastery_streak (Optional[int]): The longest consecutive mastery streak.
            Defaults to None.
        current_win_streak (Optional[int]): The current consecutive win streak. Defaults to None.
        current_mastery_streak (Optional[int]): The current consecutive mastery streak.
            Defaults to None.
    """
    games_played: int = 0
    games_escaped: int = 0
    games_mastered: int = 0
    games_won: int = 0
    win_rate: Optional[float] = None
    deepest_level: Optional[int] = None
    cumulative_levels: Optional[int] = None
    highest_score: Optional[int] = None
    cumulative_score: Optional[int] = None
    most_gold: Optional[int] = None
    cumulative_gold: Optional[int] = None
    most_lumenstones: Optional[int] = None
    cumulative_lumenstones: Optional[int] = None
    fewest_turns_win: Optional[int] = None
    cumulative_turns: Optional[int] = None
    longest_win_streak: Optional[int] = None
    longest_mastery_streak: Optional[int] = None
    current_win_streak: Optional[int] = None
    current_mastery_streak: Optional[int] = None
