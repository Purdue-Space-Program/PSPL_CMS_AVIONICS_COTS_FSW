FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    gpiod \
    libgpiod-dev \
    libgtk-3-dev \
    ninja-build \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    && rm -rf /var/lib/apt/lists/*

# Clone and install dependencies
RUN git clone https://github.com/cbl17/daqhats.git /daqhats
WORKDIR /daqhats
RUN ./install.sh

WORKDIR /fsw
COPY . .

RUN cmake -S . -B build -G Ninja
RUN cmake --build build --target install

FROM scratch AS export-stage
COPY --from=builder /usr/local/bin/fsw /usr/local/bin/fsw
