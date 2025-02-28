#!/bin/bash

docker build -t psp-cms-fsw .
docker run -it -v $PWD:/work -w /work psp-cms-fsw bash