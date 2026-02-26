---
id: doc-flat-hierarchy
type: pattern
tags: [documentation, organization]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Flat Hierarchy

# Pattern: One-Deep Singular Directories

To maintain a flat and easily traversable knowledge graph, documentation must follow a singular depth rule.

## Structure
- **Root Pillar:** e.g., `docs/developer/`
- **Singular Category:** e.g., `pattern/`
- **Atomic File:** e.g., `this-file.md`

## Constraints
- **Forbidden:** Nested sub-directories within category folders (e.g., `docs/developer/pattern/auth/jwt.md` is invalid).
- **Required:** Use expressive, prefixed filenames to differentiate categories (e.g., `auth-jwt.md`).
