---
id: doc-system
type: module
pillar: architecture
---
[Home](/) > [Architecture](/docs/architecture/readme.md) > [Module](readme.md) > Doc System

# Module: Doc System

The Documentation-as-Code (DaC) engine governing the repository's knowledge graph structure.

## 1. Physical Scope
- **Path:** `/docs/`
- **Ownership:** All Contributors

## 2. Capability Alignment
- [Capability: Governance Enforcement](/docs/architecture/capability/governance-enforcement.md) (T2)

## 3. Pattern Composition
- [doc-flat-hierarchy](/docs/developer/pattern/doc-flat-hierarchy.md) (P) — Flat, prefix-named file structure
- [doc-yaml-metadata](/docs/developer/pattern/doc-yaml-metadata.md) (P) — RAG-optimized frontmatter
- [doc-breadcrumb-navigation](/docs/developer/pattern/doc-breadcrumb-navigation.md) (P) — Line-6 breadcrumb on all docs
- [doc-structured-readme](/docs/developer/pattern/doc-structured-readme.md) (P) — Manifest readmes with 10+ items

## 4. Telemetry & Observability
- **Probe:** `ai.doc.sync` — validates knowledge graph integrity
- **Status:** 🔲 Not yet instrumented
