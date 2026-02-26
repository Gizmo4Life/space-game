---
id: architecture-review
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Architecture Review

## 1. Objective
Ensure structural alignment with the repository's T1 (Landscape), T2 (Capability), and T3 (Module) hierarchy.

## 2. Structural Alignment
- **Verify:** Adhere to [Pillar Ownership](/docs/developer/pattern/doc-pillar-ownership.md).
- **Verify:** The new code/doc belongs to an existing [T2 Capability].
- **Verify:** The new files are mapped to a [T3 Module].
- **Action:** If no mapping exists, execute [Discovery Protocol](discovery.md) to define new architectural artifacts.


## 3. Domain Integrity
- **Verify:** [Module Dependencies](/docs/developer/pattern/doc-module-dependency.md) are explicitly documented.

- **Action:** If structural drift is detected, update the corresponding Landscape or Capability artifact.
