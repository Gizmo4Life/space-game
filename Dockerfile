# Build Stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    gpg \
    wget \
    software-properties-common \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libxrandr-dev \
    libxcursor-dev \
    libudev-dev \
    libfreetype6-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libdrm-dev \
    libgbm-dev \
    libwayland-dev \
    libxkbcommon-dev \
    wayland-protocols \
    pkg-config \
    libxinerama-dev \
    libxi-dev \
    libcurl4-openssl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Build Box2D 3.1.0 (Native CMake Export Support)
RUN git clone --depth 1 --branch v3.1.0 https://github.com/erincatto/box2d.git /tmp/box2d && \
    cd /tmp/box2d && \
    mkdir -p build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF && \
    make -j$(nproc) && \
    make install

# Build SFML 3.0
RUN git clone --depth 1 --branch 3.0.0 https://github.com/SFML/SFML.git /tmp/sfml && \
    cd /tmp/sfml && \
    mkdir -p build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DSFML_BUILD_EXAMPLES=OFF -DSFML_BUILD_DOC=OFF && \
    make -j$(nproc) && \
    make install

# Install EnTT (Header-only)
RUN git clone --depth 1 --branch v3.13.0 https://github.com/skypjack/entt.git /tmp/entt && \
    cd /tmp/entt && \
    mkdir -p build && cd build && \
    cmake .. && \
    make install

# Build nlohmann-json (CMake config required by OTel export)
RUN git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git /tmp/nlohmann_json && \
    cd /tmp/nlohmann_json && \
    mkdir -p build && cd build && \
    cmake .. -DJSON_BuildTests=OFF && \
    make install

# Build/Install OpenTelemetry C++ (v1.14.2)
RUN git clone --depth 1 --branch v1.14.2 https://github.com/open-telemetry/opentelemetry-cpp.git /tmp/otel && \
    cd /tmp/otel && \
    mkdir -p build && cd build && \
    cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DWITH_OTLP_HTTP=ON -DWITH_OTLP_GRPC=OFF -DBUILD_TESTING=OFF -DWITH_EXAMPLES=OFF && \
    make -j$(nproc) && \
    make install

WORKDIR /app
COPY . .

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Final Stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libfreetype6 \
    libopenal1 \
    libflac8 \
    libvorbisenc2 \
    libvorbisfile3 \
    libgl1-mesa-glx \
    libxrandr2 \
    libxcursor1 \
    libudev1 \
    libcurl4 \
    libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/SpaceGame .
COPY --from=builder /usr/local/lib/libsfml-* /usr/local/lib/
COPY --from=builder /usr/local/lib/libbox2d* /usr/local/lib/
COPY --from=builder /usr/local/lib/libopentelemetry* /usr/local/lib/

# Update ldconfig to find the new libraries
RUN ldconfig

ENV DISPLAY=host.docker.internal:0
CMD ["./SpaceGame"]
