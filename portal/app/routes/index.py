# In portal/app/routes/index.py
from flask import Blueprint, render_template

# Register index routes under the Blueprint
bp = Blueprint('index', __name__)

@bp.route('/', methods=['GET'])
def index():
    """
    Root endpoint for the API.
    Renders the Tracker Landing Page (index.html).
    """
    return render_template('index.html')
