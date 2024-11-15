from flask import Flask, request, jsonify
from prometheus_client import Gauge, generate_latest, CONTENT_TYPE_LATEST
import prometheus_client as prom

app = Flask(__name__)

# Define Prometheus metrics
player_gold = Gauge('brogue_player_gold', 'Amount of gold collected by the player')
player_depth = Gauge('brogue_depth_level', 'Current depth level of the player')
player_hp = Gauge('brogue_player_current_hp', 'Current hit points of the player')
player_turns = Gauge('brogue_player_turns', 'Total turns played by the player')

# Endpoint to receive metrics
@app.route('/metrics', methods=['POST', 'GET'])
def receive_metrics():
    if request.method == 'POST':
        data = request.json
        if not data:
            return jsonify({"error": "No JSON payload received"}), 400

        # Update metrics with the received values
        player_gold.set(data.get('gold', 0))
        player_depth.set(data.get('depth', 0))
        player_hp.set(data.get('hp', 0))
        player_turns.set(data.get('turns', 0))

        return jsonify({"status": "success", "received": data}), 200
    elif request.method == 'GET':
        # Return the current values of each metric in JSON format
        return jsonify({
            "player_gold": player_gold._value.get(),
            "player_depth": player_depth._value.get(),
            "player_hp": player_hp._value.get(),
            "player_turns": player_turns._value.get()
        }), 200
    
# Prometheus scrape endpoint
@app.route('/metrics/prometheus', methods=['GET'])
def prometheus_metrics():
    return generate_latest(), 200, {'Content-Type': CONTENT_TYPE_LATEST}

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)