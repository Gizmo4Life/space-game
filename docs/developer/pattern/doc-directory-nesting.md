---
id: doc-directory-nesting
type: pattern
tags: [anti-pattern, structure]
category: anti-pattern
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Directory Nesting

# Anti-pattern: Sub-directory Nesting

Creating deep, nested folder structures within the `docs/` pillar.

## Why it's Forbidden
- **Path Complexity:** Increases the risk of broken relative links.
- **Discovery Friction:** Humans and agents must traverse multiple layers to find related content.

## Corrective Action
Adopt a [Flat Hierarchy](/docs/developer/pattern/doc-flat-hierarchy.md) using expressive file prefixes (e.g., `auth-jwt.md`) instead of folders.
