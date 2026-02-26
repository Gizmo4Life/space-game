---
id: doc-pillar-ownership
type: pattern
tags: [architecture, governance]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Pillar Ownership

## Structure
- **Rule:** Every code file and documentation artifact must belong to exactly one Pillar (Architecture, External, Developer, Governance, Operations).
- **Rule:** Cross-pillar dependencies must be explicitly declared and verified.
- **Verify:** No "Ghost Files" (files without a governing manifest entry).
