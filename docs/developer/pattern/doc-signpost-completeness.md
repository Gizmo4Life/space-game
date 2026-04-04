---
id: doc-signpost-completeness
type: pattern
pillar: developer
category: geometry
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Signpost Completeness

# Pattern: Signpost Completeness

## 1. Problem
A documentation subdirectory without a `readme.md` signpost is invisible to navigational tooling and agent discovery. Parent readmes that omit subdirectories from their listing create orphaned branches in the Knowledge Graph.

## 2. Solution
Every subdirectory under `docs/` that contains markdown files **must** have a `readme.md` signpost. Every parent directory's readme **must** list all child subdirectories in both its human-readable **Sub-directories** section and its **Machine Navigation Metadata** YAML block.

### Signpost Readme Template
```markdown
---
id: {pillar}-{subdir}-manifest
type: manifest
pillar: {pillar}
---
[Home](/) > [Docs](/docs/readme.md) > [{Pillar}](/docs/{pillar}/readme.md) > {Title}

# Sub-pillar: {Title}

{One-line description of what this directory contains.}

## Contents
- [item-a](item-a.md): Description.
- [item-b](item-b.md): Description.
```

### Parent Listing Requirement
```yaml
# In parent readme.md Machine Navigation Metadata:
index_map:
  child_dir:
    path: child_dir/
    scope: Description of directory purpose.
```

## 3. Constraints
- No subdirectory may exist under `docs/` without a `readme.md`.
- The parent readme must list the subdirectory in both prose and YAML metadata.
- This applies recursively — if `docs/a/b/` exists, both `docs/a/readme.md` and `docs/a/b/readme.md` must exist.

## 4. Traceability
- **Standard:** [Document Organization](/docs/governance/standard/document-organization.md) (P)
- **Related:** [signpost-readme](signpost-readme.md)
