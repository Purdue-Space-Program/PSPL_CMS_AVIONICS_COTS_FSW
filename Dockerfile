FROM --platform=linux/arm64 debian:bookworm

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    gpiod \
    libgpiod-dev \
    ninja-build

RUN apt-get install -y bear
RUN apt-get install -y clangd
RUN apt-get install -y ripgrep
RUN apt-get install -y libboost-all-dev
RUN apt-get install -y clang
RUN apt-get install -y git-lfs
RUN apt-get install -y libc++-dev libc++abi-dev
RUN apt-get install -y lldb gdb
