#!/usr/bin/env sh

echo "arduino-cli board attach"

arduino-cli board attach

echo "arduino-cli compile --build-path './build' --output-dir './artifacts' $@"

arduino-cli compile --build-path './build' --output-dir './artifacts' "$@"