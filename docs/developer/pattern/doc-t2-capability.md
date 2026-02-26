---
id: doc-t2-capability
type: pattern
tags: [meta, architecture, orchestration]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc T2 Capability

## Structure
- **YAML Frontmatter:** Must include `type: capability`.
- **1. Business Intent:** Defines the high-level goal and stakeholders.
- **2. Orchestration Flow:** A numbered list mapping business steps to specific [T3 Modules].
- **3. Data Flow & Integrity:** Defines triggers, outputs, and consistency models (e.g., ACID).
- **4. Operational Context:** Links to the primary [Runbook] and defines the critical failure metric.