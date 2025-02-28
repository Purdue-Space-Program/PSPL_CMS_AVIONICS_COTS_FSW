FROM --platform=linux/arm64 ubuntu:22.04 

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    gpiod \
    libgpiod-dev \
    ninja-build

RUN groupadd -g 1000 builder && \
useradd -u 1000 -g builder -m builder

USER builder
