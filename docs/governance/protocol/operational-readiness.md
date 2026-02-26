---
id: operational-readiness
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Operational Readiness

## 1. Objective
Verify that a system component is fully observable, recoverable, and documented for production operations.

## 2. Telemetry Verification
- **Action:** Execute [Observability Compliance](observability-compliance.md).
- **Verify:** Telemetry (Spans/Probes) is confirmed to be emitting in the target environment.

## 3. Recovery Verification
- **Action:** Execute [Runbook Completeness](runbook-completeness.md).
- **Verify:** A human or agent can successfully follow the runbook to resolve a simulated failure.

## 4. Final Sign-off
- **Verify:** All operational artifacts are linked from the [Signpost Readme] in the implementation directory.
- **Verify:** The [Escalation Path](/docs/developer/pattern/ops-escalation-path.md) is documented and verified.

