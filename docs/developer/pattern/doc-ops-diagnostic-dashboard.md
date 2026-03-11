---
id: doc-ops-diagnostic-dashboard
3: type: pattern
4: tags: [operational, observability, dashboard]
5: ---
6: [Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Diagnostic Dashboard
7: 
8: ## 1. Objective
9: To provide a structured, machine-readable definition of operational dashboards in SigNoz, enabling rapid troubleshooting of T2 Capabilities.
10: 
11: ## 2. Structure
12: - **YAML Frontmatter:** Must include `type: diagnostic_dashboard`, `capability: [T2-ID]`, and `tool: SigNoz`.
13: - **1. Dashboard Context:** Link to the [T2 Capability] this dashboard monitors.
14: - **2. Key Performance Indicators (KPIs):**
15:   - Define the PromQL or ClickHouse queries used for the primary charts.
16:   - Example: `rate(signoz_latency_bucket{serviceName="SpaceGame", spanName="engine.physics.step"}[5m])`
17: - **3. Alert Thresholds:**
18:   - List critical triggers that define a "System Unhealthy" state.
19: - **4. Signal-to-Runbook Mapping:**
20:   - For every major KPI outlier, link to the corresponding [Span Runbook](/docs/developer/pattern/doc-ops-span-runbook.md).
21: 
22: ## 3. Lifecycle
23: Dashboards must be updated during the **Validation** phase of the [Unified Change Protocol](/docs/governance/protocol/unified-change.md) for any change affecting a P-rated Capability.
