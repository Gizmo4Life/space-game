---
id: ops-documentation
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Operational Documentation

## Context: Incident Response & System State
*Nuance: Operational documentation must optimize for speed of mitigation during an active incident. High-density, query-ready files are strictly prioritized over narrative.*

| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [doc-ops-alert](/docs/developer/pattern/doc-ops-alert.md) | **P** | 1:1 mapping with platform alerts. | N/A |
| [doc-ops-unified-runbook](/docs/developer/pattern/doc-ops-unified-runbook.md) | **P** | Primary System-wide entry point. | N/A |
| [doc-ops-span-runbook](/docs/developer/pattern/doc-ops-span-runbook.md) | **P** | Atomic span mitigation layer. | N/A |
| [doc-ops-restoration-step](/docs/developer/pattern/doc-ops-restoration-step.md) | **P** | Idempotent CLI/scripts for recovery. | N/A |
| [ops-triage-path](/docs/developer/pattern/ops-triage-path.md) | **P** | Priority-based issue routing. | N/A |
| [ops-escalation-path](/docs/developer/pattern/ops-escalation-path.md) | **P** | Defined tier-up guidelines. | N/A |
| [ops-scoped-resource-discovery](/docs/developer/pattern/ops-scoped-resource-discovery.md) | **P** | Bounded context for investigative queries. | N/A |
| [doc-ops-monolithic-runbook](/docs/developer/pattern/doc-ops-monolithic-runbook.md) | **D** | Use only for legacy discovery phases. | N/A |
| [doc-ops-unverified-mitigation](/docs/developer/pattern/doc-ops-unverified-mitigation.md) | **U** | Mitigation without a verification query. | N/A |
