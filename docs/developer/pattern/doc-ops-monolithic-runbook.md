---
id: doc-ops-monolithic-runbook
type: pattern
tags: [anti-pattern, operational]
category: anti-pattern
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Monolithic Runbook

# Anti-pattern: Monolithic Runbook

Creating single, large recovery documents that mix triage, diagnostics, and multiple restoration paths.

## Why it's Forbidden
- **High MTTR:** Operators must scroll through irrelevant content during high-pressure incidents.
- **Merge Conflicts:** Multiple contributors editing the same large file creates friction.

## Corrective Action
Bifurcate into the [Universal System Runbook](/docs/developer/pattern/doc-ops-unified-runbook.md) hierarchy with atomic [Span Runbooks](/docs/developer/pattern/doc-ops-span-runbook.md).
