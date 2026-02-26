---
id: arch-documentation
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Architecture Documentation

## Context: System Design & Mapping
*Nuance: Architecture documentation must cleanly separate business intent from code mechanics. Mixing these creates fragile documentation that breaks on every code refactor.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-t1-landscape](/docs/developer/pattern/doc-t1-landscape.md) | **P** | Required at the root of `docs/architecture/` to orient agents globally. |
| [doc-t2-capability](/docs/developer/pattern/doc-t2-capability.md) | **P** | Required for defining business logic sequences. Max 100 lines. |
| [doc-t3-module](/docs/developer/pattern/doc-t3-module.md) | **P** | Required for physical code mapping. Must link directly to `/src` signposts. Max 50 lines. |
| **T2 with Code Snippets** | **U** | Unacceptable. T2 must remain implementation-agnostic. |
| **T3 detailing Business Logic** | **U** | Unacceptable. T3 must only describe structural patterns and telemetry. |