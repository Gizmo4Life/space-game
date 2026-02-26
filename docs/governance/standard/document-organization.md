---
id: document-organization
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Document Organization

## Context: Repository File System
*Nuance: Prioritize machine-readability (RAG) and flat, collision-free hierarchies over deep human categorization.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-breadcrumb-navigation](/docs/developer/pattern/doc-breadcrumb-navigation.md) | **P** | Required at the top of every documentation file. |
| [doc-flat-hierarchy](/docs/developer/pattern/doc-flat-hierarchy.md) | **P** | Ensures a flat graph (e.g., `docs/developer/pattern/`). |
| [doc-yaml-metadata](/docs/developer/pattern/doc-yaml-metadata.md) | **P** | Required on all files for RAG metadata indexing. |
| [doc-dichotomy](/docs/developer/pattern/doc-dichotomy.md) | **P** | Patterns define "What." Standards define "Should." |
| **Sub-directory Nesting** | **U** | Forbidden. Use expressive prefixes (e.g., `auth-jwt.md`) instead. |
| **Paragraphs > 2 Sentences in READMEs** | **U** | Forbidden. Use Mermaid diagrams or bifurcate into atomic files. |