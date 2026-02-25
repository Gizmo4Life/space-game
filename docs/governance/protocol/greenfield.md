---
id: greenfield-protocol
type: protocol
pillar: governance
---
[Home](/) > [Governance] > [Protocol] > Greenfield

## 1. Objective
Add new functionality by deriving a testable solution definition, scaffolding the architecture, and executing the implementation.

## 2. Testable Solution Definition
- **Action:** Define the [T2 Capability](/docs/developer/pattern/doc-t2-capability.md) outlining the business intent and orchestration flow.
- **Action:** Define the exact acceptance criteria, telemetry expectations, or unit tests required to prove the capability functions.
- **Verify:** User approval of the T2 Capability and test definition *before* proceeding.

## 3. Architectural Scaffolding
- **Action:** Materialize the hollow [T3 Modules](/docs/developer/pattern/doc-t3-module.md).
- **Action:** Select the **Preferred (P)** [Patterns] from the [Standard Manifest] that will be used to construct the code.

## 4. Implementation Execution
- **Action:** Write the physical code in `/src` adhering strictly to the selected Patterns.
- **Action:** Embed the [Signpost Readme] in the new directories.
- **Verify:** Execute the test/verification defined in Step 2.