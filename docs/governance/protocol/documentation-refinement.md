---
id: documentation-refinement
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Documentation Refinement

## 1. Objective
Iteratively extract undocumented patterns from the codebase, refine standards to evaluate their fitness, and update architectural documentation to reference them — repeating until the documentation is complete and atomic.

## 2. Pre-flight
- **Action:** Verify access to the pattern index (`docs/developer/pattern/readme.md`), standards (`docs/governance/standard/`), and architecture modules (`docs/architecture/module/`).
- **Verify:** No phantom references (README links to non-existent files) or orphan patterns (files missing from the index).

## 3. Extraction Pass
- **Action:** Scan the contents of the repository for structural shapes not yet in the pattern index.
- **Action:** For each candidate, apply the [Pattern Intake Protocol](pattern-intake.md):
  1. Decontextualize (strip domain-specific variables).
  2. Materialize (create atomic pattern file — structure only, no "should/must" language).
  3. Shadow Index (add to `docs/developer/pattern/readme.md` and `index_map`).
- **Verify:** Each new pattern file contains YAML frontmatter, breadcrumb, and only structural definition.

## 4. Standards Refinement
- **Action:** For each new or existing pattern lacking contextual evaluation, identify or create a PADU standard in `docs/governance/standard/`.
- **Action:** Insert the pattern into the appropriate PADU table with a fitness rating (P/A/D/U) and contextual nuance.
- **Action:** Add counter-patterns (D/U entries) to complete the fitness spectrum.
- **Verify:** Every P-rated pattern in the standard has at least one D or U counter-pattern.

## 5. Architecture Update
- **Action:** For each new pattern, identify the T2 Capability and T3 Module docs that manifest the pattern.
- **Action:** Add the pattern to the module's **Pattern Composition** section with its PADU rating.
- **Action:** If the T2 Capability doc's **Data Flow & Integrity** section lacks pattern references, update it.
- **Verify:** No module has patterns in its source code that are absent from its Pattern Composition section.

## 6. Integrity Check
- **Action:** Cross-reference:
  - Pattern files on disk vs. README index entries (zero phantoms, zero orphans).
  - Pattern README `index_map` keys (no duplicates).
  - Standards referencing patterns that exist on disk.
- **Action:** If gaps remain, return to Step 3.
- **Exit Condition:** All three layers (patterns → standards → architecture) are consistent and atomic.
