#!/bin/bash
export VSOMEIP_CONFIGURATION=/home/zz/car/build/config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=seat_test_client
echo "Starting Seat Service Client Test..."
echo "Configuration: $VSOMEIP_CONFIGURATION"
echo "Application: $VSOMEIP_APPLICATION_NAME"
echo "Press Ctrl+C to exit"
/home/zz/car/build/bin/test_seat_client
