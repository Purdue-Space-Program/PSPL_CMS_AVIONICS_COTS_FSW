FROM --platform=linux/arm64 ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

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

# Copy application source code
WORKDIR /fsw
COPY . .

# Configure and build the project
RUN cmake -S . -B build -G Ninja && \
    cmake --build build --target install

# Stage 2: Extract only the compiled binary
FROM scratch AS export-stage
COPY --from=builder /fsw/install/bin/fsw ./
