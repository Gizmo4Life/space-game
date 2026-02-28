---
id: documentation-validation
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Documentation Validation

## 1. Objective
Identify structural errors, missing artifacts, and technical drift between the repository's code and its Documentation-as-Code (DaC) state.

## 2. Structural Audit
- **Action:** Verify [doc-breadcrumb-navigation](/docs/developer/pattern/doc-breadcrumb-navigation.md) is present on line 6 of all `.md` files in `docs/`.
- **Action:** Verify every file has a valid YAML frontmatter with `id` and `type`.
- **Action:** Verify all internal links (e.g., `file:///...`) are functional and point to existing files.

## 3. Drift & Omission Analysis
- **Action:** Scan for directories in `src/` or `tools/` that lack a corresponding [T3 Module] in `docs/architecture/module/`.
- **Action:** Scan for [T3 Modules] that lack a [Signpost Readme] in their physical code directory.
- **Action:** Verify every [T3 Module] explicitly links to at least one [Standard] and its [Preferred] patterns.
- **Action:** For each [T3 Module], spot-check that every **named class or system** in the module's physical scope (`.h`/`.cpp` files) is described under a **Key Systems** or dedicated sub-section. A system mentioned only in a file path or passing reference is **not** considered documented. Flag any undescribed class as a **content gap**.

## 4. Operational & Logic Audit
- **Action:** Scan for critical telemetry spans in code that lack a corresponding [doc-ops-span-runbook](/docs/developer/pattern/doc-ops-span-runbook.md).
- **Action:** Identify "Ghost Logic"—defined as code blocks that implement patterns marked as **Unacceptable (U)** or **Discouraged (D)** in the standards.

## 5. Verification
- **Verify:** No dead links remain in the Knowledge Graph.
- **Verify:** The `docs/readme.md` root manifest correctly maps all child pillars.
