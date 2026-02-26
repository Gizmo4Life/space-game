---
id: cicd-reproducible-build
type: pattern
tags: [cicd, infrastructure, reproducibility]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > CICD Reproducible Build

## Structure
- **Requirement:** Build scripts must be versioned alongside the code they build.
- **Requirement:** Avoid global dependencies; all tools must be managed via local package managers (npm, pip, etc.).
- **Verify:** Use a clean environment (e.g., Docker or ephemeral CI runner) for every build.
