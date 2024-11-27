import os

class Config:
    """Base configuration."""
    DEBUG = False
    TESTING = False
    SECRET_KEY = os.getenv("SECRET_KEY", "default-secret-key")
    PORTAL_NAMESPACE = os.getenv("PORTAL_NAMESPACE", "portal")
    
    MONSTER_CRD_GROUP = "kaschaefer.com"
    MONSTER_CRD_VERSION = "v1"
    MONSTER_CRD_PLURAL = "monsters"

    PROMETHEUS_METRICS_PATH = "/metrics"
    PROMETHEUS_PORT = 5000
