"""
This module defines the `index` blueprint for handling the root endpoint of the application.

The blueprint registers a route for the root URL ('/') that renders the index page of the 
application.

Returns:
    None: This module does not return values directly but defines routes for the Flask application.
"""
from flask import Blueprint, render_template

# Register index routes under the Blueprint
bp = Blueprint('index', __name__)

@bp.route('/', methods=['GET'])
def index():
    """
    Root endpoint for the API.

    This route is responsible for rendering the landing page of the application, 
    which is typically the homepage or the first page users interact with. It uses 
    Flask's `render_template` function to render the `index.html` template, which 
    contains the front-end interface or introductory content for the application.

    Returns:
        Response: The rendered HTML template (index.html).
    """
    return render_template('index.html')  # Render and return the index.html page

