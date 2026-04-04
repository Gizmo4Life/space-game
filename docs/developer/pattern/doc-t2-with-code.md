---
id: doc-t2-with-code
type: pattern
tags: [anti-pattern, architecture]
category: geometry
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > T2 with Code

# Anti-pattern: T2 with Code Snippets

Mixing implementation details (code snippets) into high-level business logic definitions (T2).

## Why it's Forbidden
- **Fragility:** T2 documents should describe "What" the system does. Code snippets make them brittle and prone to drift during refactors.
- **Context Overload:** Agents and humans need clean abstractions to understand business intent without getting bogged down in mechanics.

## Corrective Action
Move all code snippets to atomic [T3 Modules](/docs/developer/pattern/doc-t3-module.md) or [Restoration Steps](/docs/developer/pattern/doc-ops-restoration-step.md).
