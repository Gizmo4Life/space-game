---
id: signpost-readme
type: pattern
tags: [meta, navigation, linkage]
pillar: developer
---
[Home](/) > [Developer] > [Pattern] > Signpost Readme

## Structure
- **Location:** The `readme.md` file embedded inside any non-docs implementation directory (e.g., `/src/billing/readme.md`).
- **Header:** Contains absolute paths to the [Home] and the specific governing [Standard].
- **Linkage:** Contains exactly one hard-link to the corresponding architectural [T3 Module] or operational [Task].
- **Local Context:** Contains ephemeral terminal commands (e.g., `npm run test:billing`).