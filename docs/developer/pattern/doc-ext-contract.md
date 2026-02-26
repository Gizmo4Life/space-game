---
id: doc-ext-contract
type: pattern
tags: [meta, external, api]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ext Contract

## Structure
- **YAML Frontmatter:** Must include `type: contract`.
- **1. Interface Scope:** Public vs. Partner vs. Internal.
- **2. Schema Reference:** Link to the raw `.json` or `.yaml` OpenAPI/Protobuf file.
- **3. Versioning Strategy:** How breaking changes are handled.
- **4. SLA:** Expected uptime and latency for this contract.