---
id: doc-monolithic-wiki
type: pattern
tags: [anti-pattern, structure]
category: anti-pattern
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Monolithic Wiki

# Anti-pattern: Monolithic Wiki Page

Mixing orchestration (T2), implementation (T3), and operational steps into a single large file.

## Why it's Forbidden
- **RAG Inefficiency:** Large files exceed token limits for granular agent retrieval.
- **Ownership Ambiguity:** Hard to assign specific owners to sub-sections of a monolithic document.

## Corrective Action
Explode the document into atomic files using the [Standard Topologies](/docs/governance/standard/document-topologies.md).
