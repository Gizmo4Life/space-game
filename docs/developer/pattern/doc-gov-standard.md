---
id: doc-gov-standard
type: pattern
tags: [meta, governance, logic]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Doc Gov Standard

## Structure
- **YAML Frontmatter:** Must include `type: standard`.
- **1. Context:** The specific environment or constraint (e.g., "Public API").
- **2. Nuance:** A prioritized list of values (e.g., "Security > Latency").
- **3. The Matrix:** A Markdown table with columns:
  - **Pattern:** Link to a [Pattern] file.
  - **Rating:** P / A / D / U.
  - **Reasoning:** Context-specific justification.