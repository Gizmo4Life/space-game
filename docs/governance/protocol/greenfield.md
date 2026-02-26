---
id: greenfield-protocol
type: protocol
pillar: governance
---
[Home](/) > [Governance] > [Protocol] > Greenfield

## 1. Objective
Add new functionality by deriving a testable solution definition, scaffolding the architecture, and executing the implementation.

## 2. Planning & Design
- **Action:** Execute [Test Planning](test-planning.md) to define acceptance criteria.
- **Action:** Execute [Architecture Review](architecture-review.md) to map the new functionality.
- **Action:** Execute [Observability Compliance](observability-compliance.md) to plan telemetry emission.
- **Verify:** User approval of the T2 Capability and test definition *before* proceeding.

## 3. Implementation Execution
- **Action:** Materialize the hollow [T3 Modules](/docs/developer/pattern/doc-t3-module.md).
- **Action:** Select the **Preferred (P)** [Patterns] from the [Standard Manifest].
- **Action:** Write the physical code in `/src` adhering strictly to the selected Patterns.
- **Action:** Embed the [Signpost Readme] in the new directories.
- **Verify:** Execute the test/verification defined in Step 2.