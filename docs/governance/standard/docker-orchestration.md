---
id: docker-orchestration-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Docker Orchestration

# Standard: Docker Orchestration

Establish the technical foundations for containerized development, build reproducibility, and observability orchestration.

| Pattern / Behavior | Rating | Nuance |
| :--- | :--- | :--- |
| [docker-multi-stage-build](/docs/developer/pattern/docker-multi-stage-build.md) | **P** | Required to minimize image size and attack surface. |
| [docker-source-build](/docs/developer/pattern/docker-source-build.md) | **P** | Mandatory for SFML 3.0+, Box2D 3.1+, and OTel. |
| [docker-kitware-cmake](/docs/developer/pattern/docker-kitware-cmake.md) | **P** | Required for C++20/23 compliance (CMake 3.24+). |
| [docker-host-isolation](/docs/developer/pattern/docker-host-isolation.md) | **P** | Mandatory prevents host absolute path leaks (CMakeCache collision). |
| [docker-shared-library-enforcement](/docs/developer/pattern/docker-shared-library-enforcement.md) | **P** | Prevents linkage mismatches (Shared/Static) in containers. |
| [docker-transitive-dependency-management](/docs/developer/pattern/docker-transitive-dependency-management.md) | **A** | Strategy for OTel/JSON leaks; internal implementations preferred. |
| **Apt-Only Installation** | **D** | Discouraged for core dependencies due to Ubuntu repository lag. |
| **Obsolete Compose Tags** | **U** | `version: "3.x"` is obsolete in modern Compose. |

## 1. Build & Dependency Management
- **Multi-Stage Builds**: All image builds MUST use a `builder` stage for compilation and a separate `runtime` stage for execution (See [Multi-Stage Build Pattern](/docs/developer/pattern/docker-multi-stage-build.md)).
- **Source Pinning**: Core dependencies (SFML, Box2D, EnTT) must be built from source using specific Git tags (See [Source Build Pattern](/docs/developer/pattern/docker-source-build.md)).
- **Isolation**: Use `.dockerignore` to ensure host build state does not leak (See [Host Isolation Pattern](/docs/developer/pattern/docker-host-isolation.md)).

## 2. Orchestration & Connectivity
- **Healthchecks**: Services with dependencies (e.g., `game` depending on `clickhouse`) must use `service_healthy` conditions for reliable startup.
- **X11 Bridge**: Local development on macOS mandates the use of XQuartz and the `DISPLAY=host.docker.internal:0` environment variable.
- **Observability**: The OTel Collector is the primary aggregator. Services must emit OTLP to the collector endpoint rather than individual backends.

## 3. Success State (Definition of Done)
- `docker compose build` completes without errors on Ubuntu 22.04.
- `protocol-turbo.sh` passes all infrastructure checks.
- `docker compose ps` shows all services as `Up` or `Healthy`.
- Telemetry is successfully visible in SigNoz upon startup.
