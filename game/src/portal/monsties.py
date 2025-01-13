
from flask import Flask, jsonify

app = Flask(__name__)

# ...existing code...

new_monsties = ["monstie1", "monstie2", "monstie3"]
monsties = []

@app.route('/monsties/new', methods=['GET'])
def get_new_monsties():
    global new_monsties, monsties
    response = {"pod-names": new_monsties.copy()}
    monsties.extend(new_monsties)
    new_monsties.clear()
    return jsonify(response)

# ...existing code...

if __name__ == '__main__':
    app.run(debug=True)