# ============================================================================
# SpaceGame Dockerfile — Multi-Stage Build
# See: docs/governance/standard/docker-orchestration.md
# See: docs/developer/pattern/docker-source-build.md
# ============================================================================

# ---------------------------------------------------------------------------
# Stage 1: Builder — compile all dependencies and the application
# ---------------------------------------------------------------------------
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# -- 1a. System packages & Kitware CMake repo (3.24+ required) ---------------
# Pattern: docker-kitware-cmake
RUN apt-get update && apt-get install -y \
    gpg \
    wget \
    software-properties-common \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    # SFML build deps
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
    # Box2D / GLFW transitive deps (Wayland + X11)
    libwayland-dev \
    libxkbcommon-dev \
    wayland-protocols \
    pkg-config \
    libxinerama-dev \
    libxi-dev \
    # OTel build deps
    libcurl4-openssl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# -- 1b. Source-built dependencies -------------------------------------------
# Pattern: docker-source-build, docker-shared-library-enforcement
# IMPORTANT: Use mkdir -p build (pattern: docker-host-isolation)
# IMPORTANT: Use -DBUILD_SHARED_LIBS=ON (pattern: docker-shared-library-enforcement)

# Box2D 3.1.0 — Must be >= 3.1 for native CMake export support
RUN git clone --depth 1 --branch v3.1.0 https://github.com/erincatto/box2d.git /tmp/box2d && \
    cd /tmp/box2d && \
    mkdir -p build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DBOX2D_SAMPLES=OFF -DBOX2D_UNIT_TESTS=OFF && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/box2d

# SFML 3.0.0
RUN git clone --depth 1 --branch 3.0.0 https://github.com/SFML/SFML.git /tmp/sfml && \
    cd /tmp/sfml && \
    mkdir -p build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DSFML_BUILD_EXAMPLES=OFF -DSFML_BUILD_DOC=OFF && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/sfml

# EnTT v3.13.0 (header-only — install only, no compile)
RUN git clone --depth 1 --branch v3.13.0 https://github.com/skypjack/entt.git /tmp/entt && \
    cd /tmp/entt && \
    mkdir -p build && cd build && \
    cmake .. && \
    make install && \
    rm -rf /tmp/entt

# nlohmann-json v3.11.3 — CMake config required by OTel exported targets
# Pattern: docker-transitive-dependency-management
RUN git clone --depth 1 --branch v3.11.3 https://github.com/nlohmann/json.git /tmp/nlohmann_json && \
    cd /tmp/nlohmann_json && \
    mkdir -p build && cd build && \
    cmake .. -DJSON_BuildTests=OFF && \
    make install && \
    rm -rf /tmp/nlohmann_json

# OpenTelemetry C++ v1.14.2
# NOTE: -DCMAKE_POLICY_VERSION_MINIMUM=3.5 needed for Kitware CMake compat
RUN git clone --depth 1 --branch v1.14.2 https://github.com/open-telemetry/opentelemetry-cpp.git /tmp/otel && \
    cd /tmp/otel && \
    mkdir -p build && cd build && \
    cmake .. -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DWITH_OTLP_HTTP=ON \
    -DWITH_OTLP_GRPC=OFF \
    -DBUILD_TESTING=OFF \
    -DWITH_EXAMPLES=OFF && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/otel

# -- 1c. Build the application -----------------------------------------------
WORKDIR /app
COPY . .

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# ---------------------------------------------------------------------------
# Stage 2: Runtime — minimal image with only what's needed to run
# ---------------------------------------------------------------------------
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# Runtime-only shared libraries (no -dev headers)
RUN apt-get update && apt-get install -y --no-install-recommends \
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
    libxi6 \
    libxinerama1 \
    libgl1-mesa-dri \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy only the built binary and shared libraries from builder
COPY --from=builder /app/build/SpaceGame .
COPY --from=builder /usr/local/lib/libsfml-* /usr/local/lib/
COPY --from=builder /usr/local/lib/libbox2d* /usr/local/lib/
COPY --from=builder /usr/local/lib/libopentelemetry* /usr/local/lib/

# Refresh the shared library cache
RUN ldconfig

ENV DISPLAY=host.docker.internal:0
CMD ["./SpaceGame"]
