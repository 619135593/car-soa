#!/bin/bash
export VSOMEIP_CONFIGURATION=/home/zz/car/build/config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=window_test_client
echo "Starting Window Service Client Test..."
echo "Configuration: $VSOMEIP_CONFIGURATION"
echo "Application: $VSOMEIP_APPLICATION_NAME"
echo "Press Ctrl+C to exit"
/home/zz/car/build/bin/test_window_client
