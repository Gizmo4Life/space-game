---
id: doc-breadcrumb-navigation
type: pattern
tags: [documentation, navigation, RAG]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Breadcrumb Navigation

# Pattern: Breadcrumb Navigation

To ensure a traversable knowledge graph and consistent agent navigation, every documentation file must include a standardized breadcrumb trail at the top.

## Structure

The breadcrumb must reside on line 6 (immediately following the YAML frontmatter) and follow this hierarchy:

`[Home](/) > [Docs](/docs/readme.md) > [Pillar](/docs/<pillar>/readme.md) > [Sub-pillar](readme.md) > Title`

### Components
- **Home**: Link to the repository root `/`.
- **Docs**: Link to the documentation root `/docs/readme.md`.
- **Pillar**: Link to the primary pillar root (e.g., `Architecture`, `Governance`).
- **Sub-pillar**: Link to the local `readme.md` (the manifest).
- **Title**: Plain text title of the current file.

## Constraints
- **Required**: Must be the first line of content after the frontmatter header.
- **Forbidden**: Shorthand links (e.g., `[Governance]`) without explicit paths.
- **RAG Benefit**: Enables autonomous agents to "upkeep" their context by jumping to parent manifests.
