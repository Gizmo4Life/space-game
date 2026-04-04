---
id: doc-sequential-numbering
type: pattern
pillar: developer
category: geometry
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Sequential Section Numbering

# Pattern: Sequential Section Numbering

## 1. Problem
Documents with duplicate `##` heading numbers (e.g., two `## 3.` sections) break scanability for both humans and parsers. This occurs most frequently when sections are added over time without renumbering.

## 2. Solution
All `##` headings within a document must use strictly sequential numbering starting at 1. Sub-headings (`###`) under a numbered section may use unnumbered descriptive titles or scoped numbering (e.g., `### A.`, `### B.`).

### Correct
```markdown
## 1. Physical Scope
## 2. Capability Alignment
## 3. Key Systems
## 4. Pattern Composition
## 5. Telemetry & Observability
```

### Incorrect
```markdown
## 1. Physical Scope
## 2. Capability Alignment
## 3. Key Systems
## 3. Pattern Composition   ← Duplicate
## 4. Telemetry             ← Looks correct but follows a gap
```

## 3. Constraints
- No two `##` headings in a single file may share the same number.
- Numbering must be contiguous — no gaps (e.g., jumping from `## 2.` to `## 5.`).
- When inserting a new section, all subsequent sections must be renumbered.

## 4. Traceability
- **Standard:** [Document Organization](/docs/governance/standard/document-organization.md) (P)
