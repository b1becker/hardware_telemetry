#!/usr/bin/env bash
set -euo pipefail

echo "Starting Deployment\n"
cmake --build build --clean-first
echo "Rebuilt Cmake"
sudo systemctl restart hardware-telemetry
sudo systemctl status hardware-telemetry
