---
id: getting-started
type: workflow
tags: [onboarding, setup]
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Workflow](readme.md) > Getting Started

## 1. Objective
To initialize a local development environment that passes the [Discovery Protocol](/docs/governance/protocol/discovery.md).

## 2. Quick Start (Docker - Recommended)
The fastest way to launch the game with full observability (SigNoz/Jaeger) is via Docker Compose.

1.  **Prerequisites**: Install [Docker Desktop](https://www.docker.com/products/docker-desktop/).
2.  **Display (macOS)**: Install [XQuartz](https://www.xquartz.org/) and run `xhost +localhost`.
3.  **Launch**:
    ```bash
    docker-compose up --build -d
    ```
4.  **Access**:
    - **Game**: Window will launch via X11.
    - **SigNoz (Dashboards)**: [http://localhost:3301](http://localhost:3301)
    - **Jaeger (Traces)**: [http://localhost:16686](http://localhost:16686)

## 3. Alternative: Manual Build (macOS)
Use this for low-latency development or if Docker is unavailable.

1.  **Prerequisites**:
    - **Compiler**: Clang/GCC (C++20).
    - **Build System**: CMake >= 3.24.
2.  **Install Dependencies**: 
    ```bash
    brew install sfml box2d entt opentelemetry-cpp
    ```
3.  **Configure & Build**:
    ```bash
    mkdir build && cd build
    cmake ..
    make -j$(sysctl -n hw.ncpu)
    ```
4.  **Launch**: `./SpaceGame`

## 4. Definition of Done
- `./SpaceGame` (or container) launches successfully.
- Telemetry spans are visible in **SigNoz** or **Jaeger**.
- Documentation adheres to the [Unified Change Protocol](/docs/governance/protocol/unified-change.md).