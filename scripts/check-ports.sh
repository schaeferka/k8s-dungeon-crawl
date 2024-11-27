#!/bin/bash

# Ensure the .env file exists
if [ ! -f "../.env" ]; then
    node ./logger.js ".env file not found. Please make sure it exists in the correct directory."
    exit 1
fi

# Load environment variables
source ../.env

# Function to check if a port is in use
check_port() {
    local port=$1
    if lsof -i ":$port" &>/dev/null; then
        return 1  # Port is in use
    else
        return 0  # Port is available
    fi
}

# Check each port individually and prompt for action if any are in use
for port in $KDC_LOCAL_PORT_8090 $KDC_LOCAL_PORT_5910 $KDC_LOCAL_PORT_8010 $KDC_LOCAL_PORT_5000 $KDC_LOCAL_PORT_9090 $KDC_LOCAL_PORT_3000; do
    if ! check_port $port; then
        echo "Port $port is already in use."
        echo "Please free up the port."
        echo "The port is in use by the following process: \$(lsof -i :$port)"
        echo "You can free up the port by killing the process using the port with: kill -9 \$(lsof -t -i:$port)"
        exit 1
    fi
done

echo "All specified ports are available."