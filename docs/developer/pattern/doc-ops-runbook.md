---
id: doc-ops-runbook
type: pattern
tags: [meta, operational, incident]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Runbook

## Structure
- **YAML Frontmatter:** Must include `type: runbook`.
- **1. Symptom:** The specific alert or metric deviation (e.g., `latency > 1s`).
- **2. Severity:** SEV-1 (Critical) to SEV-4 (Trivial).
- **3. Impact:** Who is affected? (e.g., "Checkout unavailable").
- **4. Diagnostic Phase:** Queries to isolate the failing [T3 Module].
- **5. Mitigation Phase:** Immediate actions to stop the bleeding (e.g., Rollback).
- **6. Restoration Phase:** Steps to return to full health.
- **7. Escalation:** Who to call if Mitigation fails.