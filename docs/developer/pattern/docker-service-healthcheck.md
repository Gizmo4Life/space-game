---
id: docker-service-healthcheck
type: pattern
pillar: developer
---

# Pattern: Docker Service Healthcheck

## Problem
Services depending on ClickHouse fail to start because the healthcheck (`wget --spider localhost:8123/ping`) never passes — the HTTP interface is unreliable or slow on Alpine-based images.

## Solution
Use the native client binary for healthchecks instead of HTTP probes. Combine with `start_period` and generous retries.

## Implementation
```yaml
clickhouse:
  image: clickhouse/clickhouse-server:24.1-alpine
  healthcheck:
    test: [ "CMD", "clickhouse-client", "--query", "SELECT 1" ]
    interval: 5s
    timeout: 5s
    retries: 15
    start_period: 10s
```

## Rules
1. **Use native clients** (`clickhouse-client`, `pg_isready`, `redis-cli ping`) over HTTP probes.
2. **Set `start_period`** to allow slow cold-starts (database recovery, WAL replay).
3. **Dependent services** MUST use `depends_on: { condition: service_healthy }`.

## Nuance
- **Rating: Preferred (P)** — Native checks are faster and more reliable than HTTP probes in minimal images.
