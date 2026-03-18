---
description: Iterative "Ralph Wiggum Loop" for fulfilling the Definition of Done (DoD).
---

# Workflow: Ralph Wiggum Loop (RWL)

This workflow is used by sub-agents to iteratively execute changes until the **[Definition of Done](/docs/developer/pattern/definition-of-done.md)** is met. It starts from an approved **[Implementation Plan](/docs/developer/pattern/definition-of-done.md#3-mandatory-artifacts)** and ends with a Pull Request to `main`.

## 1. Setup
// turbo
1. Create a feature branch from `main` with a descriptive name: `git checkout -b feature/[task-id]-[short-desc]`.
2. Ensure you have the **Implementation Plan** and **Success Criteria** from the Primary Agent.

## 2. Gap Identification
1. Compare the current state of the codebase and documentation against the **Definition of Done (DoD)**.
2. Identify the first gap (Prioritize: **Tests** and **Documentation** first).
   - *Example:* "Need unit tests for telemetry spans."
   - *Example:* "Need T3 module documentation update."

## 3. The Loop (Iterative Refinement)
Repeat the following steps for each identified gap until DoD is 100% satisfied:

1. **Address Gap:** Implement the missing logic, test, or documentation fragment.
2. **Build:** Run `cmake --build build` to ensure zero compilation errors.
3. **Test:** Run all tests related to the change. Ensure zero failures.
4. **Lint:** Run linter checks on modified files. Fix all violations.
5. **Verify DoD:** Re-evaluate the remaining gaps against the **Definition of Done**.

## 4. Completion
1. Perform a final **[Documentation Validation](/docs/governance/protocol/documentation-validation.md)**.
2. Perform a final **[Architecture Review](/docs/governance/protocol/architecture-review.md)** to ensure topological integrity.
3. Commit all changes with a clear description of the solution.
4. Push the branch and submit a PR to `main`.
5. Provide the PR link to the user.

> [!IMPORTANT]
> The Ralph Wiggum Loop is a compliance-driven process. Never bypass a DoD requirement (like telemetry or runbooks) for speed. If a requirement is unclear, return to the **Primary Agent** for elicitation.
