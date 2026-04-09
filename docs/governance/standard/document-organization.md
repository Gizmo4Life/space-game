---
id: document-organization
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Document Organization

## 1. Documentation Trichotomy
To ensure clear separation of concerns, all new documentation must adhere strictly to one of the following layers:
- **Architecture (`docs/architecture/`):** Defines *WHAT* patterns are physically aggregated to achieve a system capability or module. Must contain context, not technical implementations.
- **Standard (`docs/governance/standard/`):** Defines *WHEN* a pattern is preferred over an acceptable alternative. Contains normative governance and PADU preference tables.
- **Pattern (`docs/developer/pattern/`):** Defines *HOW* to implement the strategy in code. Contains code snippets, definitions, and technical constraints.

## Context: Repository File System
*Nuance: Prioritize machine-readability (RAG) and flat, collision-free hierarchies over deep human categorization.*

| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [doc-breadcrumb-navigation](/docs/developer/pattern/doc-breadcrumb-navigation.md) | **P** | Top of every documentation file. | N/A |
| [doc-flat-hierarchy](/docs/developer/pattern/doc-flat-hierarchy.md) | **P** | Flat graph structure. | N/A |
| [doc-yaml-metadata](/docs/developer/pattern/doc-yaml-metadata.md) | **P** | RAG metadata indexing core. | N/A |
| [doc-category-tag](/docs/developer/pattern/doc-category-tag.md) | **P** | Every pattern file must declare its category in YAML frontmatter. | N/A |
| [doc-sequential-numbering](/docs/developer/pattern/doc-sequential-numbering.md) | **P** | All `##` headings must use strictly sequential numbering. | N/A |
| [doc-signpost-completeness](/docs/developer/pattern/doc-signpost-completeness.md) | **P** | Every subdirectory must have a readme and be listed in its parent. | N/A |
| [doc-structured-readme](/docs/developer/pattern/doc-structured-readme.md) | **P** | Required for manifests with 10+ items. | N/A |
| [signpost-readme](/docs/developer/pattern/signpost-readme.md) | **P** | Short redirect files in source folders mapping to docs. | N/A |
| [doc-walkthrough](/docs/developer/pattern/doc-walkthrough.md) | **P** | End-to-end trace of a completed workflow. | N/A |
| [doc-gov-standard](/docs/developer/pattern/doc-gov-standard.md) | **P** | Normative standard with PADU rating table. | N/A |
| [doc-dichotomy](/docs/developer/pattern/doc-dichotomy.md) | **P** | Splitting definition (pattern) from fitness (standard). | N/A |
| [doc-pillar-ownership](/docs/developer/pattern/doc-pillar-ownership.md) | **P** | Top-level semantic alignment (dev, architecture, governance). | N/A |
| [doc-module-dependency](/docs/developer/pattern/doc-module-dependency.md) | **P** | Explicit topological mapping between architectural modules. | N/A |
| [doc-directory-nesting](/docs/developer/pattern/doc-directory-nesting.md) | **U** | Nested folders (use prefixes instead). | N/A |
| [readme-long-prose](/docs/developer/pattern/readme-long-prose.md) | **U** | READMEs with narrative paragraphs. | N/A |
