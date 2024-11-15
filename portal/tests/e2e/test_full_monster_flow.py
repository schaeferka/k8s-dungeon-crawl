import pytest
from flask import Flask
from app import create_app
from app.routes.monsters import monsters_data  # Assuming you're using in-memory storage

@pytest.fixture
def app():
    # Setup the app for testing
    app = create_app(config_name="development")
    yield app  # Returns the app instance to the test

@pytest.fixture
def client(app):
    # Setup the test client
    return app.test_client()

@pytest.fixture(autouse=True)
def reset_monsters_data():
    """Reset the in-memory monsters data before each test."""
    monsters_data.clear()  # Assuming monsters_data is the in-memory data structure

def test_monster_creation_flow(client):
    # Create a new monster via the POST route
    response = client.post('/monsters/', json=[{
        "id": 1,
        "name": "Dragon",
        "depth": 2,
        "status": "alive",
        "spawned_at": "2024-11-13T12:00:00Z",
        "type": "fire"
    }])
    
    # Check if the response status is 201 (Created)
    assert response.status_code == 200
    assert b"Dragon" in response.data

    # Verify the monster appears in the list (GET /monsters/data)
    response = client.get('/monsters/data')
    assert response.status_code == 200

    # Check if the response data contains the newly created monster
    monsters = response.get_json()
    assert len(monsters) == 1  # Ensure we have one monster
    assert monsters[0]['name'] == "Dragon"
    assert monsters[0]['status'] == "alive"

    # Verify the monster's details
    response = client.get('/monsters/count')
    assert response.status_code == 200
    assert b"monster_count" in response.data  

def test_metrics(client):
    response = client.get('/metrics/')
    assert response.status_code == 200
    assert b'brogue_monster_count' in response.data 