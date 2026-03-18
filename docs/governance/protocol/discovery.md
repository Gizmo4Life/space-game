---
id: discovery-protocol
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Discovery

## 1. Objective
Find undocumented or outdated documentation with respect to current code, characterize the patterns, and update architectural files.

## 2. Drift Analysis (Code vs. Docs)
- **Action:** Execute [Documentation Validation](documentation-validation.md).
- **Verify:** Identify physical code files that lack a corresponding T3 module or have stale pattern mappings.
- **Verify:** Ensure T3 Module filenames match their physical directory pillar (e.g., `src/engine/` -> `engine-*`).
- **Action: Ghost Logic Audit** — For each target file, search for [Ghost Logic](/docs/developer/pattern/ghost-logic.md) signals:
  - Any `registry.view<T>()` loop that could be replaced by a call to a shared utility (e.g., `findFlagship`).
  - Any multi-step component aggregation block that is duplicated across files (e.g., manual `InstalledModules` iteration).
  - Any stat derivation that recalculates a value already computed by a dedicated system (e.g., summing module mass when `ShipStats` exists).
  - Flag all findings as D-rated candidates requiring [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) or [single-source-calculation](/docs/developer/pattern/single-source-calculation.md) refactoring.


## 3. Characterization & Elicitation
- **Action:** For newly discovered or altered code shapes, execute [Capability Elicitation](capability-elicitation.md) (Brownfield Mode).
- **Action:** Execute [Pattern Intake](pattern-intake.md).

- **Action:** Execute [Standard Definition](standard-definition.md) to assign fitness ratings.
- **Action:** Execute [Architecture Review](architecture-review.md) to reconcile the new mappings.
- **Action:** Execute [Operational Readiness](operational-readiness.md) to verify existing telemetry/recovery.

## 4. Architectural Reconciliation
- **Action:** Create or update the T3 Module files to explicitly link the discovered code to Patterns and Standards.
- **Action:** Link the T3 Module upwards to its governing T2 Capability.
- **Action:** Execute [Documentation Validation](documentation-validation.md) to reconcile the Knowledge Graph.
- **Verify:** Ensure the target functional directory contains a [Signpost Readme](/docs/developer/pattern/signpost-readme.md).