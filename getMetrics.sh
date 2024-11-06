# K8S: Script to get metrics from the Portal  
while true; do
    curl -s http://localhost:5001/metrics | jq .
    sleep 1  # Adjust the delay as needed (1 second in this case)
done