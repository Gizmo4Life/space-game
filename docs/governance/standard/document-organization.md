---
id: document-organization
type: standard
pillar: governance
---
[Home](/) > [Governance] > [Standard] > Document Organization

## Context: Repository File System
*Nuance: Prioritize machine-readability (RAG) and flat, collision-free hierarchies over deep human categorization.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| **One-Deep Singular Directories** | **P** | Ensures a flat graph (e.g., `docs/developer/pattern/`). |
| **YAML Frontmatter / Index Maps** | **P** | Required on all files for RAG metadata indexing. |
| **Pattern/Standard Dichotomy** | **P** | Patterns define "What." Standards define "Should." |
| **Sub-directory Nesting** | **U** | Forbidden. Use expressive prefixes (e.g., `auth-jwt.md`) instead. |
| **Paragraphs > 2 Sentences in READMEs** | **U** | Forbidden. Use Mermaid diagrams or bifurcate into atomic files. |