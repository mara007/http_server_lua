#!/usr/bin/env bash

while true; do
    # curl localhost:20000/$1 -v
    wget localhost:20000/$1  -O /dev/null
done

