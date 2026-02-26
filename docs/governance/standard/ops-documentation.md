---
id: ops-documentation
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Operational Documentation

## Context: Incident Response & System State
*Nuance: Operational documentation must optimize for speed of mitigation during an active incident. High-density, query-ready files are strictly prioritized over narrative.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-ops-alert](/docs/developer/pattern/doc-ops-alert.md) | **P** | Must map 1:1 with observability platform alerts. |
| [doc-ops-unified-runbook](/docs/developer/pattern/doc-ops-unified-runbook.md) | **P** | Required triage layer. Must map directly to a T2 Capability. |
| [doc-ops-span-runbook](/docs/developer/pattern/doc-ops-span-runbook.md) | **P** | Required restoration layer. Must map directly to a failing T3 Module span. |
| [doc-ops-task](/docs/developer/pattern/doc-ops-task.md) | **P** | Required for atomic execution. Must be perfectly idempotent CLI/scripts. |
| **Monolithic Runbook** | **D** | Discouraged. Acceptable only during legacy discovery before splitting into Unified/Span levels. |
| **Unverified Mitigation Scripts** | **U** | Unacceptable. Tasks must include a verification query. |