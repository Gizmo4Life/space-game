---
id: doc-ops-unified-runbook
type: pattern
tags: [meta, operational, triage]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Unified Runbook

## Structure
- **YAML Frontmatter:** Must include `type: unified_runbook` and `capability: [T2-ID]`.
- **1. Capability Scope:** Link to the [T2 Capability] this runbook protects.
- **2. Dashboard Orientation:** Links to the primary observability dashboards (e.g., Grafana, Datadog) used to visualize the flow.
- **3. Triage Matrix:** A table mapping specific failing telemetry spans to their corresponding detailed [Span Runbooks].
  - *Example:* If `billing.auth.fail` spikes -> Go to `[Auth Span Runbook]`.
- **4. Global Escalation:** Who to page if multiple spans are failing simultaneously (cascading failure).