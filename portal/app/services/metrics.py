"""
This module defines the `MetricsService` class, which is responsible for managing and updating
Prometheus metrics related to the game, including player stats, game events, and monster data.

It uses the `prometheus_client` library to expose various metrics such as the amount of gold
collected by the player, the player's current depth level, the number of games played, and monster 
creation and death data.

The class also integrates logging to track metric updates and events.

Prometheus Metrics:
- `brogue_player_gold`: Amount of gold collected by the player.
- `brogue_depth_level`: Current depth level of the player.
- `brogue_player_current_hp`: Current hit points of the player.
- `brogue_games_played`: Total number of games played.
- `brogue_games_won`: Total number of games won.
- `brogue_monster_count`: Total number of monsters created.
- `brogue_monster_death_count`: Total number of monsters that have died.
- `brogue_monster_lifespan_seconds`: Time (in seconds) that monsters stay alive.
- `brogue_last_monster_created_timestamp`: Timestamp of the last monster created.
- `brogue_last_monster_death_timestamp`: Timestamp of the last monster death.

This service is designed to update and track the game's key metrics in real-time.

Returns:
    None: This module does not return any values.
"""
import time
import logging
from prometheus_client import Gauge, Counter, Histogram
from typing import Dict

# Configure logging
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
handler = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
logger.addHandler(handler)

class MetricsService:
    """
    A service class to manage Prometheus metrics for the game.

    This class provides methods to update and track various metrics related to the player,
    game events, and monsters. It uses Prometheus client libraries to define and update gauges, 
    counters, and histograms for tracking in-game statistics. Metrics are logged for monitoring
    purposes.

    Attributes:
        player_gold (Gauge): Tracks the amount of gold collected by the player.
        player_depth (Gauge): Tracks the player's current depth level in the game.
        player_current_hp (Gauge): Tracks the player's current health points.
        games_played (Counter): Tracks the total number of games played.
        games_won (Counter): Tracks the total number of games won.
        monster_count (Counter): Tracks the total number of monsters created.
        monster_death_count (Counter): Tracks the total number of monster deaths.
        monster_lifespan_histogram (Histogram): Tracks how long monsters stay alive in the game.
        last_monster_created (Gauge): Tracks the timestamp of the last monster created.
        last_monster_death (Gauge): Tracks the timestamp of the last monster death.

    Methods:
        update_player_metrics: Updates the player's metrics based on provided data.
        record_game_event: Records a game event such as "played" or "won".
        record_monster_creation: Records the creation of a new monster.
        record_monster_death: Records the death of a monster and updates related metrics.
        reset_metrics: Resets all metrics, typically for starting a new game.
    """

    def __init__(self):
        """
        Initialize Prometheus metrics for the game.

        Sets up all the required Prometheus metrics for tracking player data, game events, 
        and monster stats. This method configures gauges, counters, and histograms for the 
        different game metrics, and logs the successful initialization of metrics.

        Raises:
            None
        """
        # Player Metrics
        self.player_gold = Gauge('brogue_player_gold', 'Amount of gold collected by the player')
        self.player_depth = Gauge('brogue_depth_level', 'Current depth level of the player')
        self.player_current_hp = Gauge('brogue_player_current_hp',
                                       'Current hit points of the player')

        # Game Metrics
        self.games_played = Counter('brogue_games_played', 'Total games played')
        self.games_won = Counter('brogue_games_won', 'Total games won')

        # Monster Metrics
        self.monster_count = Counter('brogue_monster_count', 'Total number of monsters created')
        self.monster_death_count = Counter('brogue_monster_death_count',
                                           'Total number of monsters that have died')
        self.monster_lifespan_histogram = Histogram(
            'brogue_monster_lifespan_seconds', 
            'Time (in seconds) monsters stay alive',
            buckets=[10, 30, 60, 300, 600, 1800, 3600, 7200]
        )
        self.last_monster_created = Gauge('brogue_last_monster_created_timestamp',
                                          'Timestamp of the last monster creation')
        self.last_monster_death = Gauge('brogue_last_monster_death_timestamp',
                                        'Timestamp of the last monster death')

        logger.info("Initialized Prometheus metrics.")

    def update_player_metrics(self, data: Dict[str, float]) -> None:
        """
        Update Prometheus player metrics.

        Updates the player's metrics such as gold collected, depth level, and current health points.
        The method checks if the relevant data keys exist in the provided dictionary and updates the
        corresponding Prometheus metrics.

        Args:
            data (Dict[str, float]): A dictionary containing player metrics (e.g., gold, depth, HP).

        Raises:
            None
        """
        if 'gold' in data:
            self.player_gold.set(data['gold'])
        if 'depthLevel' in data:
            self.player_depth.set(data['depthLevel'])
        if 'currentHP' in data:
            self.player_current_hp.set(data['currentHP'])
        logger.info("Updated player metrics: %s", data)

    def record_game_event(self, event: str) -> None:
        """
        Record a game-related event in Prometheus metrics.

        This method increments the respective game event counters based on the provided event type
        ('played' or 'won').

        Args:
            event (str): The type of game event ('played', 'won').

        Raises:
            None
        """
        if event == 'played':
            self.games_played.inc()
        elif event == 'won':
            self.games_won.inc()
        logger.info("Recorded game event: %s", event)

    def record_monster_creation(self, monster_id: str) -> None:
        """
        Record the creation of a new monster.

        This method increments the monster creation counter and logs the timestamp of the last 
        monster created.

        Args:
            monster_id (str): Unique identifier for the monster.

        Raises:
            None
        """
        self.monster_count.inc()
        self.last_monster_created.set_to_current_time()
        logger.info("Recorded monster creation: %s", monster_id)

    def record_monster_death(self, monster_id: str, creation_time: float) -> None:
        """
        Record the death of a monster and update metrics.

        This method calculates the lifespan of the monster based on the creation time, increments
        the monster death counter, records the monster's lifespan, and logs the timestamp of the
        last monster death.

        Args:
            monster_id (str): Unique identifier for the monster.
            creation_time (float): Timestamp when the monster was created.

        Raises:
            None
        """
        lifespan = time.time() - creation_time
        self.monster_death_count.inc()
        self.monster_lifespan_histogram.observe(lifespan)
        self.last_monster_death.set_to_current_time()
        logger.info("Recorded monster death: %s (lifespan: %.2f seconds)", monster_id, lifespan)

    def reset_metrics(self) -> None:
        """
        Reset all metrics, typically for a new game.

        This method sets all tracked metrics (e.g., player gold, health, and depth level) to zero,
        effectively resetting the metrics for a new game session.

        Args:
            None

        Raises:
            None
        """
        self.player_gold.set(0)
        self.player_depth.set(0)
        self.player_current_hp.set(0)
        logger.info("Metrics reset for a new game.")
