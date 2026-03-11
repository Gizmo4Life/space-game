---
id: doc-ops-global-health-dashboard
type: diagnostic_dashboard
capability: global-system-health
tool: SigNoz
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Dashboard](readme.md) > Global Health

# Diagnostic Dashboard: Global System Health

## 1. Dashboard Context
Consolidated view of all critical P-rated signals: **Rendering**, **Combat**, **Economy**, **NPCs**, and **World Generation**. This unified "Single Pane of Glass" serves as the primary system health monitor.

> [!TIP]
> **SigNoz Import**: Use the physical [JSON Manifest](global-health-dashboard.json) to import this unified dashboard directly into SigNoz.

## 2. Key Performance Indicators (KPIs)

### A. Rendering Stability (T2)
- **Signal**: P99 Latency for `render.update`.
- **Query**:
  ```sql
  SELECT
    quantileAppox(0.99)(durationNano) / 1000000 AS p99_latency_ms
  FROM signoz_index.traces
  WHERE serviceName = 'SpaceGame' AND name = 'render.update'
  GROUP BY time
  ```
- **Threshold**: Red if > 16.6ms (Below 60 FPS).

### B. Combat Efficiency (T2)
- **Signal**: P99 Latency for `combat.weapon.fire`.
- **Query**:
  ```sql
  SELECT
    quantileAppox(0.99)(durationNano) / 1000000 AS p99_latency_ms
  FROM signoz_index.traces
  WHERE serviceName = 'SpaceGame' AND name = 'combat.weapon.fire'
  GROUP BY time
  ```
- **Threshold**: Yellow if > 5ms.

### C. Economic Reliability (T2)
- **Signal**: Transaction Error Rate for `economy.buy_ship`.
- **Query**:
  ```sql
  SELECT
    countIf(statusCode = 2) * 100.0 / count(*) AS error_rate_percent
  FROM signoz_index.traces
  WHERE serviceName = 'SpaceGame' AND name = 'economy.buy_ship'
  GROUP BY time
  ```
- **Threshold**: Red if > 0%.

### D. NPC Mission Throughput (T2)
- **Signal**: Successful mission outcomes per minute.
- **Query**:
  ```sql
  SELECT
    count(*) AS outcomes_per_min
  FROM signoz_index.traces
  WHERE serviceName = 'SpaceGame' AND name = 'game.npc.mission.outcome' AND attributes['mission.success'] = true
  GROUP BY time
  ```
- **Threshold**: Yellow if < 1 per 5 mins.

### E. World Generation Performance (T2)
- **Signal**: P95 Duration of top-level World Load.
- **Query**:
  ```sql
  SELECT
    quantileAppox(0.95)(durationNano) / 1000000 AS p95_load_time_ms
  FROM signoz_index.traces
  WHERE serviceName = 'SpaceGame' AND name = 'game.core.world.load'
  GROUP BY time
  ```
- **Threshold**: Warning if > 1000ms.

### F. Universe Density & Variety (T2)
- **Signal**: Quantity of bodies and station units.
- **Query**:
  ```sql
  SELECT
    countIf(name = 'game.world.gen.celestial_body') AS total_bodies,
    countIf(name = 'game.world.gen.economy_seed') AS total_stations
  FROM signoz_index.traces
  WHERE serviceName = 'SpaceGame'
  GROUP BY time
  ```
- **Threshold**: Critical if total_bodies < 5 per system.

## 3. Alert Thresholds
| KPI | Warning | Critical |
| :--- | :--- | :--- |
| **Render Latency** | > 12ms | > 16.6ms |
| **Combat Latency** | > 3ms | > 5ms |
| **Economy Errors** | > 0% | > 1% |
| **NPC Throughput** | < 1/5m | 0 per 10m |
| **World Gen Time** | > 800ms | > 1500ms |

## 4. Signal-to-Runbook Mapping
| Signal | Targeted Runbook |
| :--- | :--- |
| **Render Spike** | [render.update Runbook](/docs/operational/span/render-update.md) |
| **Combat Lag** | [combat.weapon.fire Runbook](/docs/operational/span/combat-weapon-fire.md) |
| **Transaction Failure** | [economy.buy_ship Runbook](/docs/operational/span/economy-buy-ship.md) |
| **Mission Stagnation**| [npc.mission.stagnation Runbook](/docs/operational/span/npc-mission-stagnation.md) |
| **Stockpile Imbalance**| [economy.stockpile.imbalance Runbook](/docs/operational/span/economy-stockpile-imbalance.md) |
| **World Gen Anomaly** | [world.gen.anomaly Runbook](/docs/operational/span/world-gen-anomaly.md) |
