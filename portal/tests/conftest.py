import pytest
from app import create_app

@pytest.fixture
def app():
    # Create a test instance of the Flask app
    app = create_app(config_name="development")
    yield app  # This ensures the app is available for the client fixture

@pytest.fixture
def client(app):
    # Returns the Flask test client
    return app.test_client()
