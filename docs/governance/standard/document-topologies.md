---
id: document-topologies
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Document Topologies

## Context: Repository Knowledge Graph
*Nuance: To support RAG and Agent Context Windows, documentation must be strictly typed, atomic, and heavily cross-linked.*

| Pattern | Rating | Contextual Nuance (Constraints & Limits) |
| :--- | :--- | :--- |
| [doc-t2-capability](/docs/developer/pattern/doc-t2-capability.md) | **P** | Use to map business logic. Must not contain code snippets. |
| [doc-t3-module](/docs/developer/pattern/doc-t3-module.md) | **P** | Use to map physical files to patterns. Max 50 lines. |
| [doc-ops-task](/docs/developer/pattern/doc-ops-task.md) | **P** | Use for atomic fixes. Must be perfectly idempotent. Max 50 lines. |
| [doc-gov-protocol](/docs/developer/pattern/doc-gov-protocol.md) | **P** | Use for system modifications. Max 50 lines. |
| **Monolithic Wiki Page** | **U** | Forbidden. Do not mix T2 orchestration with T3 implementation in the same file. |
| **Narrative Paragraphs** | **U** | Forbidden. Exceeding 2 sentences per paragraph breaks RAG density. Use lists or diagrams. |