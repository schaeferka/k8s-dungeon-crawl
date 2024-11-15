from prometheus_client import Gauge, Counter, Histogram
import time
import logging

# Configure logging
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


class MetricsService:
    def __init__(self):
        """
        Initialize Prometheus metrics for the game.
        """
        # Player Metrics
        self.player_gold = Gauge('brogue_player_gold', 'Amount of gold collected by the player')
        self.player_depth = Gauge('brogue_depth_level', 'Current depth level of the player')
        self.player_current_hp = Gauge('brogue_player_current_hp', 'Current hit points of the player')

        # Game Metrics
        self.games_played = Counter('brogue_games_played', 'Total games played')
        self.games_won = Counter('brogue_games_won', 'Total games won')

        # Monster Metrics
        self.monster_count = Counter('brogue_monster_count', 'Total number of monsters created')
        self.monster_death_count = Counter('brogue_monster_death_count', 'Total number of monsters that have died')
        self.monster_lifespan_histogram = Histogram(
            'brogue_monster_lifespan_seconds', 
            'Time (in seconds) monsters stay alive',
            buckets=[10, 30, 60, 300, 600, 1800, 3600, 7200]
        )
        self.last_monster_created = Gauge('brogue_last_monster_created_timestamp', 'Timestamp of the last monster creation')
        self.last_monster_death = Gauge('brogue_last_monster_death_timestamp', 'Timestamp of the last monster death')

        logger.info("Initialized Prometheus metrics.")

    def update_player_metrics(self, data: dict):
        """
        Update Prometheus player metrics.
        :param data: Dictionary containing player metrics (e.g., gold, depth, HP).
        """
        if 'gold' in data:
            self.player_gold.set(data['gold'])
        if 'depthLevel' in data:
            self.player_depth.set(data['depthLevel'])
        if 'currentHP' in data:
            self.player_current_hp.set(data['currentHP'])
        logger.info("Updated player metrics: %s", data)

    def record_game_event(self, event: str):
        """
        Record a game-related event in Prometheus metrics.
        :param event: The type of game event ('played', 'won').
        """
        if event == 'played':
            self.games_played.inc()
        elif event == 'won':
            self.games_won.inc()
        logger.info("Recorded game event: %s", event)

    def record_monster_creation(self, monster_id: str):
        """
        Record the creation of a new monster.
        :param monster_id: Unique identifier for the monster.
        """
        self.monster_count.inc()
        self.last_monster_created.set_to_current_time()
        logger.info("Recorded monster creation: %s", monster_id)

    def record_monster_death(self, monster_id: str, creation_time: float):
        """
        Record the death of a monster and update metrics.
        :param monster_id: Unique identifier for the monster.
        :param creation_time: Timestamp when the monster was created.
        """
        lifespan = time.time() - creation_time
        self.monster_death_count.inc()
        self.monster_lifespan_histogram.observe(lifespan)
        self.last_monster_death.set_to_current_time()
        logger.info("Recorded monster death: %s (lifespan: %.2f seconds)", monster_id, lifespan)

    def reset_metrics(self):
        """
        Reset all metrics, typically for a new game.
        """
        self.player_gold.set(0)
        self.player_depth.set(0)
        self.player_current_hp.set(0)
        logger.info("Metrics reset for a new game.")
