---
id: doc-ops-span-runbook
type: pattern
tags: [meta, operational, restoration]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ops Span Runbook

## Structure
- **YAML Frontmatter:** Must include `type: span_runbook` and `module: [T3-ID]`.
- **1. Component Scope:** Link to the specific [T3 Module] emitting the span.
- **2. Diagnostic Mapping:** Specific CLI queries, log filters, or dashboard panels to isolate the failure mode *within* the span.
- **3. Mitigation & Restoration:** Numbered steps linking to atomic [Restoration Steps].
- **4. Recovery Verification:** The exact telemetry signal or probe response that proves the span is healthy.