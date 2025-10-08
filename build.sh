#!/bin/bash

# Build the image and give it a name (e.g., "cmake-builder")
docker build -q -t cmake-builder .

# Run the container, execute the cmake commands, and then exit
# The "build" directory will appear in your current folder.
docker run --rm \
    -v $(pwd):/work \
    -w /work \
    cmake-builder \
    bash -c "cmake -S . -B build && cmake --build build"
