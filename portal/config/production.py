from config.default import Config

class ProductionConfig(Config):
    """Production configuration."""
    SECRET_KEY = "prod-secret-key"
    DEBUG = False
    TESTING = False
    PROMETHEUS_PORT = 5000
