#!/usr/bin/env sh

default_port=`grep -i -- "default_port:" ./sketch.yaml | awk '{print $2}'`

arduino-cli monitor --describe --port "${default_port}"

echo "arduino-cli monitor --config 'baudrate=115200' --port ${default_port} --timestamp $@"

arduino-cli monitor --config 'baudrate=115200' --port "${default_port}" --timestamp "$@"