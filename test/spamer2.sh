#!/usr/bin/env bash

while true; do
    curl localhost:20000/$1 -v
    # curl localhost:20000/$1 -v  -F '= <body>HTML message</body>'

    # wget localhost:20000/$1  -O /dev/null
done

