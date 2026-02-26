---
id: runbook-completeness
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Runbook Completeness

## 1. Objective
Ensure every system component is documented for recovery and triage to minimize Downtime (MTTR).

## 2. Requirement Mapping
- **Action:** Verify the [T3 Module] has a corresponding **Unified Runbook** ([doc-ops-unified-runbook](/docs/developer/pattern/doc-ops-unified-runbook.md)).
- **Action:** Verify every critical span has a **Span Runbook** ([doc-ops-span-runbook](/docs/developer/pattern/doc-ops-span-runbook.md)).
- **Verify:** Runbooks are linked from the module's [Signpost Readme].

## 3. Verification
- **Action:** Attempt to execute the triage matrix in the runbook against simulated or real telemetry.
- **Verify:** The "Global Escalation" contact is accurate and reachable.
