---
id: greenfield-protocol
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Greenfield

## 1. Objective
Add new functionality by gathering domain context, deriving a testable solution definition, scaffolding the architecture, and executing the implementation.

## 2. Domain Context & Elicitation
- **Action:** Execute [Capability Elicitation](capability-elicitation.md) (Greenfield Mode) to refine intent.
- **Action:** Ingest the landscape of the target capability (T2) and module (T3) via existing documentation.
- **Action:** Search for relevant [Patterns](/docs/developer/pattern/) and [Standards](/docs/governance/standard/) to identify existing architectural shapes.

- **Verify:** Establish a deep understanding of the solution space and constraints BEFORE codifying scope.

## 3. Planning & Design
- **Action:** Execute [Test Planning](test-planning.md) to codify the solution space and define refined, testable bounds.
- **Action:** Execute [Architecture Review](architecture-review.md) to map the final target functionality.
- **Action:** Execute [Standard Definition](standard-definition.md) to select/assign appropriate patterns.
- **Action:** Execute [Operational Readiness](operational-readiness.md) to plan telemetry and recovery.
- **Verify:** User approval of the T2 Capability and refined test definition *before* proceeding.

## 4. Implementation Execution
- **Action:** Materialize the hollow [T3 Modules](/docs/developer/pattern/doc-t3-module.md) (Architecture).
- **Action:** Select the **Preferred (P)** [Patterns] from the [Standard] (Standard).
- **Action:** Write the physical code in `/src` adhering strictly to the selected Patterns.
- **Action:** Materialize required [Operational Artifacts] (Runbooks/Alerts).
- **Action:** Embed the [Signpost Readme] in the new directories.
- **Action:** Execute [Documentation Validation](documentation-validation.md) (Mandatory Quality Gate).
- **Verify:** Execute the test/verification and final [Operational Readiness].