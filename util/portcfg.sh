#!/bin/sh

PORT=$1
stty -F "$PORT" 115200 -echo -echok -echoe raw

