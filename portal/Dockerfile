# Start from a lightweight Python image
FROM python:3.9-slim

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy only the requirements file to install dependencies
COPY requirements.txt .

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Create a non-root user and switch to it
RUN useradd --create-home --home-dir /home/appuser appuser
USER appuser

# Copy the rest of the application code (excluding unnecessary files like .git, tests, etc.)
COPY . .

# Expose the Flask server port
EXPOSE 5000

# Set environment variables for Flask (optional but recommended)
ENV FLASK_ENV=production

# Run the Portal server
CMD ["flask", "run", "--host=0.0.0.0", "--port=5000"]
