---
id: unified-change-protocol
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Unified Change

# Protocol: Unified Change

This protocol combines the introspective power of **Discovery** with the structured implementation of **Greenfield** to ensure all changes are standard-compliant, well-documented, and fully tested.

## 1. Introspection & Elicitation
- **Action:** Execute the [Discovery Protocol](discovery.md) to map the current state of target modules and identify technical debt or documentation drift.
- **Action:** Execute the **Domain Context & Elicitation** phase from the [Greenfield Protocol](greenfield.md).
  - *Requirement:* Derive concrete, testable functional requirements for any new behavior or modified logic.
  - *Requirement:* Define the "Success State" for the change before any code is written.
- **Action:** Audit the target code against [Architecture Standards](../standard/arch-documentation.md) and [Tech Stack Standards](../standard/game-tech-stack.md) to ensure the proposed change doesn't violate core constraints.

## 2. Planning & Test Design
- **Action:** Create an **Implementation Plan** that meets all `docs/governance/standard/` and `docs/architecture/patterns/`.
- **Action:** Define **Unit Tests** for critical logic paths (e.g., economy transactions, production rolls).
- **Verify:** The plan must call out *all* necessary changes across the codebase to avoid refactor-looping.
- **Verify:** User approval of the plan and test strategy before implementation.

## 3. Execution & Documentation
- **Action:** Implement the changes according to the approved plan.
- **Action:** Update or create **T3 Module** documentation in `docs/architecture/module/` to reflect new functionality.
- **Action:** Ensure every new directory or modified functional area has a [Signpost Readme](/docs/developer/pattern/signpost-readme.md).

## 4. Validation
- **Action:** Run all unit tests and verify 100% pass rate for new logic.
- **Action:** Perform a final **Drift Analysis** to ensure documentation and code are perfectly synchronized.
- **Action:** Execute [Documentation Validation](documentation-validation.md).
