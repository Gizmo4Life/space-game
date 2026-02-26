---
id: doc-module-dependency
type: pattern
tags: [architecture, mapping]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Module Dependency

## Structure
- **Requirement:** T3 Modules must explicitly list their upstream and downstream dependencies in their [Artifact Manifest].
- **Requirement:** Use a "Dependency Map" or graph to visualize module coupling.
- **Verify:** No circular dependencies are introduced without explicit architectural exception.
