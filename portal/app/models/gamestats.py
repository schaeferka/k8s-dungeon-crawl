# app/models/gamestats.py
from pydantic import BaseModel
from typing import Optional

class GameStats(BaseModel):
    """
    Defines the schema for GameStats data received from the game.
    """
    games_played: int = 0  # Default to 0
    games_escaped: int = 0  # Default to 0
    games_mastered: int = 0  # Default to 0
    games_won: int = 0  # Default to 0
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
