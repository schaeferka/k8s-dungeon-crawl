# Start from a lightweight Python image
FROM python:3.9-slim

# Set working directory
WORKDIR /app

# Copy and install dependencies
COPY requirements.txt .
RUN pip install -r requirements.txt

# Copy the entire portal folder, including templates
COPY . .

# Expose the Flask server port
EXPOSE 5001

# Run the Portal server
CMD ["python", "portal.py"]
