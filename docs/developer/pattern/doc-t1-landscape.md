---
id: doc-t1-landscape
type: pattern
tags: [meta, architecture, system-boundary]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc T1 Landscape

## Structure
- **YAML Frontmatter:** Must include `type: landscape`.
- **1. System Scope:** A single paragraph defining the global boundaries and primary domain of the software.
- **2. Core Capabilities:** A list mapping the highest-level business functions directly to [T2 Capabilities].
- **3. External Boundaries:** Hard links to major [External Integrations] (e.g., Payment Gateways, Cloud Providers, Identity Providers).
- **4. Global Infrastructure:** The macro deployment topology (e.g., Multi-region AWS, bare-metal Kubernetes clusters).