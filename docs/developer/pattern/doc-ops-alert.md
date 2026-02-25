---
id: doc-ops-alert
type: pattern
tags: [meta, operational, observability]
---
[Home](/) > [Developer] > [Pattern] > Doc Ops Alert

## Structure
- **YAML Frontmatter:** Must include `type: alert_rule`.
- **1. Trigger Condition:** The exact PromQL, LogQL, or metric query that fires the alert.
- **2. Severity Profile:** Defines SEV-1 through SEV-4 thresholds for this specific metric.
- **3. Routing:** A hard link to the exact [Unified Runbook] that handles this alert.