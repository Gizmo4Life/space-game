---
id: cicd-isolated-build
type: pattern
tags: [foundation, cicd]
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Isolated Build

# Pattern: Isolated Build

Every build must execute in a clean, ephemeral, and isolated environment (e.g., Docker, Runner VM).

## Why it's Required
- **Reproducibility:** Prevents "it works on my machine" failures by eliminating hidden local dependencies.
- **Security:** Limits the impact of malicious build-time scripts or compromised dependencies.

## Structure
- Use a versioned `Dockerfile` or `pipeline-config`.
- Pin all tool versions (e.g., `node:18.16.0`).
- Use ephemeral secrets injected via the pipeline vault.
