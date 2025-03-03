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

RUN echo 'root:password' | chpasswd

RUN groupadd -g 1000 builder && \
useradd -u 1000 -g builder -m builder

USER builder
