---
id: docs-root-manifest
type: manifest
---
[Home](/) > Docs

# Documentation Pillars

This repository uses a Documentation-as-Code (DaC) model to govern all repository operations and code standards.

```mermaid
graph TD
    Root[docs/readme.md] --> Gov[governance/]
    Root --> Arch[architecture/]
    Root --> Dev[developer/]
    Root --> Ops[operational/]
    Root --> Ext[external/]

    Gov --> GovS[Standard]
    Gov --> GovP[Protocol]
    Dev --> DevP[Pattern]
    Dev --> DevS[Standard]
```

## Pillars

- **[Governance](/docs/governance/readme.md)**: Meta-rules, operational protocols, and architectural constraints.
- **[Architecture](/docs/architecture/readme.md)**: System topology, capabilities (T2), and module (T3) definitions.
- **[Developer](/docs/developer/readme.md)**: Code patterns, implementation standards, and SDLC workflows.
- **[Operational](/docs/operational/readme.md)**: Runbooks, deployment procedures, and observability spans.
- **[External](/docs/external/readme.md)**: Interaction contracts and API definitions for external consumers.
