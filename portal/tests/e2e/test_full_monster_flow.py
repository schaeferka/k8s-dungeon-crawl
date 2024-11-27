"""
This module contains test fixtures and functions for testing the application's routes.

It includes fixtures for setting up the app, creating a test client, and resetting in-memory data
before each test. Additionally, it provides tests to verify the monster creation flow and the 
metrics endpoint.

Fixtures:
    app (Flask): The application instance configured for testing.
    client (FlaskClient): A test client instance for simulating HTTP requests.
    reset_monsters_data (None): Clears in-memory monster data before each test.

Tests:
    test_monster_creation_flow (None): Verifies the full flow of creating a monster and 
    checking its details.
    test_metrics (None): Verifies the correct functioning of the metrics endpoint.

Returns:
    None: No return values for this module.
    
Yields:
    None: No yielded values in this module.
"""
import pytest
from app import create_app
from app.routes.monsters import monsters_data


@pytest.fixture
def app():
    """
    Set up the application for testing.

    This fixture initializes the app with the 'development' configuration and
    yields it for use in tests. The app is torn down after the test finishes.

    Returns:
        app: The application instance configured for testing.
    """
    application = create_app(config_name="development")
    yield application


@pytest.fixture
def client(application):
    """
    Create a test client for the app.

    This fixture provides a test client that simulates HTTP requests to the app.
    It is used for interacting with the app during tests.

    Args:
        application: The application instance created by the 'app' fixture.

    Returns:
        test_client: A test client instance for sending HTTP requests.
    """
    return application.test_client()


@pytest.fixture(autouse=True)
def reset_monsters_data():
    """
    Reset the in-memory monsters data before each test.

    This fixture automatically clears the monsters data in memory before
    each test is run to ensure that each test starts with a clean state.
    """
    monsters_data.clear()


def test_monster_creation_flow(test_client):
    """
    Test the full flow of creating a monster and verifying its existence.

    This test simulates the process of creating a new monster through the
    POST endpoint, then verifies its creation and checks the monster list
    via GET requests.

    Steps:
        1. Create a new monster via the POST route.
        2. Check if the response status is 200 (OK) and verify the monster's
           name in the response data.
        3. Verify that the monster appears in the list by making a GET request
           to /monsters/data.
        4. Check if the response data contains the newly created monster and
           verify its details.
        5. Verify the monster's details by making a GET request to /monsters/count
           and check the response status and data.

    Args:
        test_client: A test client instance for making HTTP requests to the application.
    """
    # Create a new monster via the POST route
    response = test_client.post('/monsters/', json=[{
        "id": 1,
        "name": "Dragon",
        "depth": 2,
        "status": "alive",
        "spawned_at": "2024-11-13T12:00:00Z",
        "type": "fire"
    }])

    # Check if the response status is 200 (OK)
    assert response.status_code == 200
    assert b"Dragon" in response.data

    # Verify the monster appears in the list (GET /monsters/data)
    response = test_client.get('/monsters/data')
    assert response.status_code == 200

    # Check if the response data contains the newly created monster
    monsters = response.get_json()
    assert len(monsters) == 1
    assert monsters[0]['name'] == "Dragon"
    assert monsters[0]['status'] == "alive"

    # Verify the monster's details
    response = test_client.get('/monsters/count')
    assert response.status_code == 200
    assert b"monster_count" in response.data


def test_metrics(test_client):
    """
    Test the metrics endpoint.

    This test verifies that the /metrics/ endpoint returns the correct status
    code and contains the expected metrics data.

    Args:
        test_client: A test client instance for making HTTP requests to the application.
    """
    response = test_client.get('/metrics/')
    assert response.status_code == 200
    assert b'brogue_monster_count' in response.data
