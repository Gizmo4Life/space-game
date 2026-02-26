---
id: ops-triage-path
type: pattern
tags: [operational, triage, incident]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > OPS Triage Path

## Structure
- **Requirement:** Map a Symptom (from Telemetry) directly to a Severity level and a specific Mitigation Runbook.
- **Rule:** Prioritize "Query-Ready" files over narrative descriptions.
- **Verify:** A responder must be able to identify the failing component (Span/Probe) within 5 minutes of alert notification.
