from flask import Flask
from app.utils.logger import configure_logger
import os

def create_app(config_name=None):
    app = Flask(__name__, static_folder='static')


    # Determine the configuration to use
    config_name = config_name or os.getenv("FLASK_CONFIG", "default")
    
    if config_name == "development":
        app.config.from_object("config.development.DevelopmentConfig")
    elif config_name == "production":
        app.config.from_object("config.production.ProductionConfig")
    else:
        app.config.from_object("config.default.Config")
    
    # Register blueprints
    from app.routes.index import bp as index_bp  # Import from index.py
    from app.routes.monsters import bp as monsters_bp  # Import from monsters.py
    from app.routes.metrics import bp as metrics_bp  # Import from metrics.py
    from app.routes.player import bp as player_bp  # Import from player.py
    from app.routes.gamestate import bp as gamestate_bp  # Import from gamestate.py
    from app.routes.gamestats import bp as gamestats_bp  # Import from gamestats.py
    from app.routes.equipped_items import bp as items_bp  # Import from items.py
    from app.routes.pack_items import bp as pack_bp  # Import from pack.py
    from app.routes.game import bp as game_bp  # Import from game
    from app.routes.monsties import bp as monsties_bp  # Import from monsties.py

    app.register_blueprint(index_bp)  # Register index blueprint
    app.register_blueprint(monsters_bp, url_prefix="/monsters")  # Register monsters blueprint
    app.register_blueprint(metrics_bp, url_prefix="/metrics")  # Register metrics blueprint
    app.register_blueprint(player_bp, url_prefix="/player")  # Register player
    app.register_blueprint(gamestate_bp, url_prefix="/gamestate")  # Register gamestate blueprint
    app.register_blueprint(gamestats_bp, url_prefix="/gamestats")  # Register gamestats blueprint
    app.register_blueprint(items_bp, url_prefix="/items")  # Register items blueprint
    app.register_blueprint(pack_bp, url_prefix="/pack")  # Register pack blueprint
    app.register_blueprint(game_bp, url_prefix="/game")  # Register game blueprint
    app.register_blueprint(monsties_bp, url_prefix="/monsties")  # Register monties

    # Setup logging (pass the app to the logger)
    configure_logger(app)  # Pass the app to the logger setup

    return app
