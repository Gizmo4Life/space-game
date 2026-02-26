---
id: operational-triage
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Operational Triage

## 1. Objective
Identify, isolate, and mitigate system failures using the repository's operational documentation.

## 2. Signal Identification
- **Action:** Analyze active alerts and identify the failing **Span**.
- **Action:** Map the Span to its parent **T3 Module**.

## 3. Protocol Execution
- **Action:** Access the **Universal System Runbook** as the primary landing page.
- **Action:** Use the Dashboard to identify the highest priority failing **Span**.
- **Action:** Deep-dive into specific **Span Runbooks** for mitigation.
- **Action:** Execute the linked **Restoration Steps** to resolve the issue.

## 4. Mitigation & Resolution
- **Action:** Execute atomic [doc-ops-task](/docs/developer/pattern/doc-ops-task.md) files to restore state.
- **Verify:** Use [Observability Compliance](observability-compliance.md) to confirm the signal has returned to baseline.
