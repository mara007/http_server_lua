#!/usr/bin/env bash

while true; do
    # curl localhost:20000/$1 -v
    # curl localhost:20000/$1 -v  -F '= <body>HTML message</body>' &
    # curl localhost:20000/$1 -v  -F parameter1=value1 -F parameter_xyz=value2 -F kokos=kokso
    curl -G -v "http://localhost:20000/data" --data-urlencode "msg=hello world" --data-urlencode "param2=kokos"


    # wget localhost:20000/$1  -O /dev/null
done


