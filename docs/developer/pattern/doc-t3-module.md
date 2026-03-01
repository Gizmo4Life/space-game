---
id: doc-t3-module
type: pattern
tags: [meta, architecture, implementation]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc T3 Module

## Structure
- **YAML Frontmatter:** Must include `type: module`.
- **1. Physical Scope:** Absolute path to the `/src` directory and ownership.
- **2. Capability Alignment:** Hard link to the parent [T2 Capability].
- **3. Key Systems:** Functional summary of primary classes and their roles.
- **4. Pattern Composition:** A list of atomic [Patterns] used.
- **5. Telemetry & Observability:** A list of emitted Semantic Spans and Health Probes.