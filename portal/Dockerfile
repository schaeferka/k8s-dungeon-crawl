# Start from a lightweight Python image
FROM python:3.9-slim

# Set environment variables
ENV PYTHONUNBUFFERED=1

# Set working directory
WORKDIR /app

# Copy only the requirements file first and install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the application code
COPY . .

# Add a non-root user and switch to it
RUN useradd -m appuser
USER appuser

# Expose the Flask server port
EXPOSE 5001

# Run the Portal server
CMD ["python3", "portal.py"]
