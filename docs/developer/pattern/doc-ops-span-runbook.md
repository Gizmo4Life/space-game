---
id: doc-ops-span-runbook
type: pattern
tags: [meta, operational, restoration]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Span Runbook

## Structure
- **YAML Frontmatter:** Must include `type: span_runbook` and `module: [T3-ID]`.
- **1. Component Scope:** Link to the specific [T3 Module] emitting the span.
- **2. Diagnostic Queries:** Specific CLI or log queries to verify the exact error state within the span.
- **3. Mitigation & Restoration:** Numbered steps linking to atomic [Ops Tasks] (e.g., `Execute task/restart-billing-pod.md`).
- **4. Verification:** The query to run to prove the `outcome=success` span has returned to baseline.