---
id: getting-started
type: workflow
tags: [onboarding, setup]
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Workflow](readme.md) > Getting Started

## 1. Objective
To initialize a local development environment with full observability (SigNoz dashboards + Jaeger traces).

## 2. Quick Start

### Prerequisites
- **Compiler**: Clang or GCC with C++20 support
- **Build System**: CMake >= 3.24
- **Docker**: [Docker Desktop](https://www.docker.com/products/docker-desktop/) (for the observability stack)

### Step 1: Build the Game (Native)
```bash
brew install sfml box2d entt opentelemetry-cpp
mkdir -p build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Step 2: Start Observability Stack (Docker)
```bash
docker compose up -d
```
This starts **ClickHouse**, **SigNoz**, **Jaeger**, and the **OTel Collector** — no game container. The game runs natively because macOS Docker Desktop cannot provide OpenGL GPU access.

### Step 3: Run the Game with Telemetry
```bash
OTEL_EXPORTER_OTLP_ENDPOINT=http://localhost:4318 ./build/SpaceGame
```

Or use the convenience script:
```bash
./scripts/launch.sh
```

### Access Dashboards
| UI | URL | Purpose |
|---|---|---|
| SigNoz | [http://localhost:3301](http://localhost:3301) | Metrics, traces, dashboards |
| Jaeger | [http://localhost:16686](http://localhost:16686) | Distributed trace search |

### Stop the Stack
```bash
docker compose down
# or
./scripts/launch.sh --stop
```

## 3. Docker Build Validation
To verify the Dockerfile compiles correctly (CI/CD or cross-platform testing):
```bash
docker compose --profile game build
```
This builds the game inside a container but does not run it. Follows the [Docker Orchestration Standard](/docs/governance/standard/docker-orchestration.md).

## 4. Telemetry Architecture
```
SpaceGame (native macOS)
    │ OTLP/HTTP
    ▼
OTel Collector (:4318)  ──► Jaeger (:16686)
    │
    ▼
ClickHouse ──► SigNoz Query Service (:3301)
```

- The game reads `OTEL_EXPORTER_OTLP_ENDPOINT` at startup (defaults to `http://localhost:4318`)
- The OTel Collector receives traces and fans them out to all backends
- SigNoz provides dashboards; Jaeger provides trace search

## 5. Definition of Done
- `./build/SpaceGame` launches successfully with a game window.
- `docker compose ps` shows all 4 observability services as `Up` or `Healthy`.
- Telemetry spans are visible in **SigNoz** or **Jaeger**.
- Documentation adheres to the [Unified Change Protocol](/docs/governance/protocol/unified-change.md).