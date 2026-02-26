---
id: doc-ops-unified-runbook
type: pattern
type: universal_runbook
tags: [meta, operational, triage]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Universal System Runbook

# Pattern: Universal System Runbook

The primary triage entry point for all system alerts. It provides a global orientation and directs the operator to specific span-level ## Structure
- **YAML Frontmatter:** Must include `type: universal_runbook`.
- **1. Severity Matrix:** A table defining SEV-1 (Critical) to SEV-4 (Trivial) thresholds for the system.
- **2. Impact Scope:** High-level description of who/what is affected at each severity level.
- **3. System Orientation:** Links to primary observability dashboards for global health visualization.
- **4. Triage Matrix:** A prioritized table mapping failing telemetry spans to [Span Runbooks].
  - **Action:** Identify the highest priority failing span from the dashboard.
- **5. Global Escalation:** Contact chain for unresolved issues or cascading failures.