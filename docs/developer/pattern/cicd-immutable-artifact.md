---
id: cicd-immutable-artifact
type: pattern
tags: [foundation, cicd]
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Immutable Artifact

# Pattern: Immutable Artifact

Once a build produces an artifact, it is never modified. The exact same byte-for-byte binary is promoted through all environments.

## Why it's Required
- **Verification Integrity:** Ensures that what was tested in Staging is exactly what runs in Production.
- **Traceability:** Every deployment is traceable to a specific, non-malleable build ID.

## Implementation
- Version artifacts by `commit-hash` or `build-number`.
- Store artifacts in a read-only registry after production promotion.
