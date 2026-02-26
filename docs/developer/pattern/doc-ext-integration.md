---
id: doc-ext-integration
type: pattern
tags: [meta, external, vendor]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Ext Integration

## Structure
- **YAML Frontmatter:** Must include `type: integration`.
- **1. Vendor Profile:** Name, Support Portal, Account Owner.
- **2. Dependency Map:** Which [T2 Capabilities] rely on this vendor?
- **3. Failure Modes:** Known outages or quirks (e.g., "Rate limits at 100/sec").
- **4. Credential Rotation:** Protocol for updating API keys.