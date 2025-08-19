#!/bin/bash
export VSOMEIP_CONFIGURATION=/home/zz/car/build/config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=light_test_client
echo "Starting Light Service Client Test..."
echo "Configuration: $VSOMEIP_CONFIGURATION"
echo "Application: $VSOMEIP_APPLICATION_NAME"
echo "Press Ctrl+C to exit"
/home/zz/car/build/bin/test_light_client
