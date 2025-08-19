#!/bin/bash
export VSOMEIP_CONFIGURATION=/home/zz/car/build/config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=web_server
echo "Starting Body Controller Web Server..."
echo "Configuration: $VSOMEIP_CONFIGURATION"
echo "Application: $VSOMEIP_APPLICATION_NAME"
echo "HTTP Server: http://localhost:8080"
echo "SSE Events: http://localhost:8080/api/events"
echo "Press Ctrl+C to exit"
/home/zz/car/build/bin/body_controller_web_server --http-port 8080
