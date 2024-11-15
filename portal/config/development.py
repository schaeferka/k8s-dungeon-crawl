from config.default import Config

class DevelopmentConfig(Config):
    """Development configuration."""
    DEBUG = True
    TESTING = True
    SECRET_KEY = "dev-secret-key"
    PROMETHEUS_PORT = 5000
