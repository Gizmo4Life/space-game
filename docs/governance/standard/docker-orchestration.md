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
| [docker-host-isolation](/docs/developer/pattern/docker-host-isolation.md) | **P** | Prevents host absolute path leaks (CMakeCache collision). |
| [docker-shared-library-enforcement](/docs/developer/pattern/docker-shared-library-enforcement.md) | **P** | Prevents linkage mismatches (Shared/Static) in containers. |
| [docker-transitive-dependency-management](/docs/developer/pattern/docker-transitive-dependency-management.md) | **A** | Strategy for OTel target leaks; source-install + pre-load. |
| [docker-service-healthcheck](/docs/developer/pattern/docker-service-healthcheck.md) | **P** | Native client healthchecks over HTTP probes for stateful services. |
| **Apt-Only Installation** | **D** | Discouraged for core deps due to Ubuntu LTS repository lag. |
| **Unpinned Image Tags** | **D** | `:latest` creates non-reproducible environments. |
| **Obsolete Compose Tags** | **U** | `version: "3.x"` is obsolete in modern Compose Specification. |

## 1. Build & Dependency Management
- **Multi-Stage Builds**: All image builds MUST use a `builder` stage for compilation and a `runtime` stage for execution (See [Multi-Stage Build](/docs/developer/pattern/docker-multi-stage-build.md)).
- **Source Pinning**: Core dependencies MUST be built from source using pinned Git tags (See [Source Build](/docs/developer/pattern/docker-source-build.md)).
- **Shared Libraries**: Source-built dependencies MUST set `-DBUILD_SHARED_LIBS=ON` (See [Shared Library Enforcement](/docs/developer/pattern/docker-shared-library-enforcement.md)).
- **Host Isolation**: `.dockerignore` MUST exclude `build/`, `CMakeCache.txt`, and IDE directories (See [Host Isolation](/docs/developer/pattern/docker-host-isolation.md)).
- **Build Cleanup**: Each source-build `RUN` layer MUST end with `rm -rf /tmp/<dep>` to reduce layer size.
- **Runtime Libraries**: The `runtime` stage MUST install all shared libraries needed at runtime (e.g., `libxi6`, `libxinerama1`). Verify with `ldd` or by running the binary.

## 2. Orchestration & Connectivity
- **Healthchecks**: Stateful services (ClickHouse) MUST use native client healthchecks (See [Service Healthcheck](/docs/developer/pattern/docker-service-healthcheck.md)).
- **Dependency Ordering**: Services MUST use `depends_on` with explicit `condition: service_healthy` or `condition: service_started`.
- **Restart Policies**: All services MUST have `restart: unless-stopped` for self-healing.
- **Pinned Images**: All service images MUST use pinned version tags (not `:latest`).
- **Port Uniqueness**: No two services may expose the same host port. OTel Collector is the canonical OTLP endpoint (4317/4318).
- **Service Configuration**: Prefer environment variables over config file mounts for third-party services (e.g., SigNoz uses `STORAGE=clickhouse`, `ClickHouseUrl=tcp://clickhouse:9000`).
- **X11 Bridge**: macOS development requires XQuartz and `DISPLAY=host.docker.internal:0`.

## 3. Observability
- **OTel Collector**: The primary aggregation point. Services emit OTLP to the collector, which fans out to backends (Jaeger, SigNoz).
- **Telemetry Endpoint**: Set via `OTEL_EXPORTER_OTLP_ENDPOINT` environment variable.

## 4. Success State (Definition of Done)
- `docker compose build` completes without errors.
- `protocol-turbo.sh --all` passes with zero errors.
- `docker compose ps` shows all services as `Up` or `Healthy`.
- Telemetry is visible in SigNoz/Jaeger upon game startup.
