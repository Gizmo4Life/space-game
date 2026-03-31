---
id: doc-ops-diagnostic-dashboard
type: pattern
tags: [operational, observability, dashboard]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Diagnostic Dashboard

## 1. Objective
To provide a structured, machine-readable definition of operational dashboards in SigNoz, enabling rapid troubleshooting of T2 Capabilities.

## 2. Structure
- **YAML Frontmatter:** Must include `type: diagnostic_dashboard`, `capability: [T2-ID]`, and `tool: SigNoz`.
- **1. Dashboard Context:** Link to the [T2 Capability] this dashboard monitors.
- **2. Key Performance Indicators (KPIs):**
  - Define the PromQL or ClickHouse queries used for the primary charts.
  - Example: `rate(signoz_latency_bucket{serviceName="SpaceGame", spanName="engine.physics.step"}[5m])`
- **3. Alert Thresholds:**
  - List critical triggers that define a "System Unhealthy" state.
- **4. Signal-to-Runbook Mapping:**
  - For every major KPI outlier, link to the corresponding [Span Runbook](/docs/developer/pattern/doc-ops-span-runbook.md).

## 3. Lifecycle
Dashboards must be updated during the **Validation** phase of the [Unified Change Protocol](/docs/governance/protocol/unified-change.md) for any change affecting a P-rated Capability.
