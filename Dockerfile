FROM --platform=linux/arm64 debian:bookworm

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    curl \
    gpiod \
    libgpiod-dev

RUN apt-get install -y ripgrep
RUN curl https://sh.rustup.rs -sSf | bash -s -- -y
