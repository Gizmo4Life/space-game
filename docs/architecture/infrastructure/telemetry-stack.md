---
id: infra-telemetry-stack
type: infrastructure
tags: [telemetry, docker, signoz, jaeger]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Infrastructure](readme.md) > Telemetry Stack

# Infrastructure: Telemetry Stack

## 1. Overview
The Space Game uses a self-hosted, Docker-orchestrated telemetry stack based on OpenTelemetry (OTel). This stack provides distributed tracing, metrics visualization, and log aggregation.

## 2. Components
| Component | Image / Source | Role |
| :--- | :--- | :--- |
| **Jaeger** | `jaegertracing/all-in-one` | Primary distributed tracing backend for developers. |
| **SigNoz Query Service** | `signoz/query-service` | Dashboarding and metrics visualization UI. |
| **ClickHouse** | `clickhouse/clickhouse-server` | High-performance columnar database for telemetry storage. |
| **OTel Collector** | `signoz/signoz-otel-collector` | Fan-out point. Receives OTLP and exports to Jaeger/SigNoz. |
| **Game (SFML/Box2D)** | Source Build (3.0.0) | The game client, built with pinned 3.0.0 dependencies. |

## 3. Communication Flow
1.  **Game Client**: Emits OTLP (HTTP/JSON) to `http://otel-collector:4318`.
2.  **OTel Collector**: 
    - Forwards traces to **Jaeger** (gRPC port 4317).
    - Forwards traces/metrics to **SigNoz Query Service** (OTLP/gRPC).
3.  **ClickHouse**: Stores all raw trace data and pre-aggregated metrics.
4.  **SigNoz UI**: Accessible at `http://localhost:3301` for dashboarding.

## 4. Diagnostics
- **Check Health**: `docker compose ps`
- **Verify ClickHouse**: `wget --spider -q localhost:8123/ping`
- **Dashboard manifests**: Located in `/docs/operational/dashboard/` for easy import.
