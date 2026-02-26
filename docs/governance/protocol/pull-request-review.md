---
id: pull-request-review
type: protocol
pillar: governance
---
[Home](/) > [Governance] > [Protocol] > PR Review

## 1. Objective
Verify that a contribution adheres to all repository standards and maintains architectural integrity.

## 2. Architectural Verification
- **Action:** Execute [Architecture Review](architecture-review.md) on all changed files.
- **Verify:** The PR includes a [Walkthrough](/docs/developer/pattern/doc-ops-task.md) of the changes made.

## 3. Verification & Compliance
- **Action:** Execute [Test Planning](test-planning.md) to verify coverage of new logic.
- **Action:** Execute [Observability Compliance](observability-compliance.md) for any structural code changes.
- **Action:** Execute [Runbook Completeness](runbook-completeness.md) if new modules or critical spans were added.

## 4. Judgment
- **Action:** Verify all code patterns match the Preferred (P) status in the [Standard Manifest].
- **Approve:** If all sub-protocols pass and the walkthrough is accurate.
