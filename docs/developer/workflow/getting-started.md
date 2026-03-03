---
id: getting-started
type: workflow
tags: [onboarding, setup]
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Workflow](readme.md) > Getting Started

## 1. Objective
To initialize a local development environment that passes the [Discovery Protocol](/docs/governance/protocol/discovery.md).

## 2. Prerequisites
- **Compiler:** Clang or GCC supporting C++20.
- **Build System:** CMake >= 3.24.
- **Dependencies:** 
  - SFML >= 3.0
  - Box2D
  - EnTT
  - OpenTelemetry C++ SDK

## 3. Procedure
1.  **Install Dependencies (macOS):**
    ```bash
    brew install sfml box2d entt opentelemetry-cpp
    ```
2.  **Initialize Build Directory:**
    ```bash
    mkdir build && cd build
    ```
3.  **Configure & Build:**
    ```bash
    cmake ..
    make -j$(sysctl -n hw.ncpu)
    ```
4.  **Run the Application:**
    ```bash
    ./SpaceGame
    ```
5.  **Observability (Optional):**
    - Run `scripts/jaeger.sh` to start a local Jaeger instance for telemetry.

## 4. Definition of Done
- `./SpaceGame` launches successfully.
- Telemetry spans are visible in Jaeger (if running).
- Documentation adheres to the [Discovery Protocol](/docs/governance/protocol/discovery.md).