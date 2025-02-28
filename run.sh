#!/bin/bash

docker build .
docker run -it --rm \
    -v $(pwd):/work \
    -w /work \
    $(docker build -q .)