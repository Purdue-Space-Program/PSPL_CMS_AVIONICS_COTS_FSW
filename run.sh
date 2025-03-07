#!/bin/bash

docker network inspect sitl_network > /dev/null 2>&1 || \
docker network create --subnet=192.168.2.0/24 sitl_network

docker build .
docker run -it --rm \
    -v $(pwd):/work \
    -w /work \
    -p 1234:1234 \
    -p 25565:25565 \
    --net sitl_network \
    --ip 192.168.2.114 \
    $(docker build -q .)
