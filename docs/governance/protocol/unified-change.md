---
id: unified-change-protocol
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Unified Change

# Protocol: Unified Change

This protocol combines the introspective power of **Discovery** with the structured implementation of **Greenfield** to ensure all changes are standard-compliant, well-documented, and fully tested.

## 0. Pre-Flight Check
Before starting any significant change, ensure the baseline environment is healthy:
- **Build Baseline**: Verify the project compiles without errors.
- **Test Baseline**: Run existing tests to ensure zero failures before local changes.
- **Lint Baseline**: Verify a clean linter state for target files.
- **Goal**: Ensure the "Definition of Done" is not compromised by pre-existing technical debt.

## 1. Introspection & Elicitation
- **Action:** Execute the [Discovery Protocol](discovery.md) to map the current state of target modules and identify technical debt or documentation drift.
- **Action:** Execute the **Domain Context & Elicitation** phase from the [Greenfield Protocol](greenfield.md).
  - *Requirement:* Derive concrete, testable functional requirements for any new behavior or modified logic.
  - *Requirement:* Define the "Success State" for the change before any code is written.
- **Action:** Audit the target code against [Architecture Standards](../standard/arch-documentation.md) and [Tech Stack Standards](../standard/game-tech-stack.md) to ensure the proposed change doesn't violate core constraints.

## 2. Planning & Test Design
- **Action:** Create an **Implementation Plan** that meets all `docs/governance/standard/` and `docs/architecture/patterns/`.
- **Action:** Audit proposed changes against [Observability Standard](../standard/observability-standard.md). Identify required telemetry spans or probes.
- **Action:** Define **Unit Tests** for critical logic paths (e.g., economy transactions, production rolls).
- **Verify:** The plan must call out *all* necessary changes across the codebase to avoid refactor-looping.
- **Verify:** The plan must follow acceptable durable patterns as defined in the [Build Resilience Standard](../standard/build-resilience.md).
- **Verify:** User approval of the plan and test strategy before implementation.

## 3. Execution & Documentation
- **Action:** Implement the changes according to the approved plan.
- **Action:** Update or create **T3 Module** documentation in `docs/architecture/module/` to reflect new functionality.
- **Action:** For UI/Frontend changes, implement **Visual Assets** (screenshots or recordings) for the `walkthrough.md`.
- **Action:** Ensure every new directory or modified functional area has a [Signpost Readme](/docs/developer/pattern/signpost-readme.md).

## 4. Validation
- **Action:** Run all unit tests and verify 100% pass rate for new logic.
- **Action:** For performance-critical systems (Physics, AI, Rendering), verify no regression against baseline metrics using internal telemetry.
- **Action:** Run the [Protocol Turbo Script](/scripts/governance/protocol-turbo.sh) to automate the build-test-fix cycle. This script enforces the [Docker Orchestration Standard](../standard/docker-orchestration.md) by detecting known infrastructure regressions (e.g., host leaks, library mismatches).
- **Action:** Perform a final **Drift Analysis** to ensure documentation and code are perfectly synchronized.
- **Action:** Execute [Documentation Validation](documentation-validation.md).

## 5. Pattern Analysis & Standard Evolution
- **Action:** Execute the **[Pattern Intake Protocol](pattern-intake.md)** for any build errors, test failures, or linter findings.
- **Action:** Identify the specific **Context** of the failure (e.g., Platform, Testability, Build Integrity), and identify the relevant context specific standard.
- **Action:** Extract both the **Failed Attempts** (Discouraged/Unstable) and the **Successful Solution** (Preferred/Alternative) as decontextualized patterns.
- **Action:** Categorize new patterns into the PADU tables of the identified relevant context specific standard.
- **Action:** Update **[Build Resilience Standard](../standard/build-resilience.md)** with any new standards.
- **Verify:** The "Definition of Done" now includes contributing back to the governance layer to prevent future regressions.

## 6. Definition of Done
A task is considered **DONE** only when:
1. **Successful Build**: The project compiles without errors on all targeted platforms.
2. **100% Test Pass Rate**: All unit, integration, and regression tests pass with zero failures.
3. **Clean Linter Findings**: All modified files have zero linter errors or warnings as defined in the [Coding Standards](../standard/coding-standards.md).
4. **Verified Documentation**: T3 modules, Signpost Readmes, and architecture diagrams are updated and pass [Documentation Validation](documentation-validation.md).
5. **Observability Assets**: Diagnostic Dashboard manifests (.md and .json export) and Span Runbooks are present, linked, and pass drift analysis.
