---
id: doc-yaml-metadata
type: pattern
tags: [documentation, RAG, metadata]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > YAML Metadata

# Pattern: YAML Frontmatter & Index Maps

All documentation files must include machine-readable metadata to facilitate RAG indexing and graph traversal.

## Structure
- **Frontmatter:** Must appear at the very top of the file using `---` delimiters.
- **Required Fields:**
  - `id`: Unique identifier for the document.
  - `type`: Category (e.g., `pattern`, `standard`, `manifest`, `protocol`).
- **Index Maps:** For manifest files, an `index_map` YAML block at the end of the file is required to enable automated traversal.
