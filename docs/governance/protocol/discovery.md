---
id: discovery-protocol
type: protocol
pillar: governance
---
[Home](/) > [Governance] > [Protocol] > Discovery

## 1. Objective
Find undocumented or outdated documentation with respect to current code, characterize the patterns, and update architectural files.

## 2. Drift Analysis (Code vs. Docs)
- **Action:** Scan all repository directories (e.g., root, tools/, workflows/, scripts/) against `docs/architecture/` (T1, T2, T3).
- **Verify:** Identify physical code files that lack a corresponding T3 module or have stale pattern mappings.

## 3. Characterization & Review
- **Action:** For newly discovered or altered code shapes, execute [Pattern Intake](pattern-intake.md).
- **Action:** Execute [Architecture Review](architecture-review.md) to reconcile the new mappings.
- **Action:** Execute [Observability Compliance](observability-compliance.md) to verify existing telemetry.

## 4. Architectural Reconciliation
- **Action:** Create or update the T3 Module files to explicitly link the discovered code to Patterns and Standards.
- **Action:** Link the T3 Module upwards to its governing T2 Capability.
- **Verify:** Ensure the target functional directory contains a [Signpost Readme](/docs/developer/pattern/signpost-readme.md).