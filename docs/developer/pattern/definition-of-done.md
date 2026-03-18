---
id: definition-of-done
type: pattern
pillar: developer
---

# Pattern: Definition of Done (DoD)

The Definition of Done ensures that every change to the repository meets a consistent bar for quality, reliability, and maintainability.

## 1. Quality Gates

| Requirement | Description | Verification |
| :--- | :--- | :--- |
| **Successful Build** | Project compiles without errors on all targeted platforms. | `cmake --build build` |
| **100% Test Pass** | All unit, integration, and regression tests pass (zero failures). | `ctest` or specific test binaries |
| **Clean Lint** | Modified files have zero linter errors or warnings. | `clang-tidy` or equivalent |
| **Verified Docs** | T3 modules, Signpost Readmes, and architecture diagrams are updated. | [Doc Validation Protocol](/docs/governance/protocol/documentation-validation.md) |
| **Atomic Docs** | Where documentation starts to get too verbose, break it down into smaller, more manageable documents. Particularly if you are defining a concept, consider extracting it into a pattern document defining what the concept is, and then link to it from the documentation that uses it. Use the [Pattern Intake Protocol](/docs/governance/protocol/pattern-intake.md) to help you with this. Similarly, use standards with the PADU scale to effectively document which patterns are to be used and avoided in different contexts. |
| **Observability** | Dashboards and Span Runbooks are provided for new logic. | Drift Analysis |
| **End User Documentation** | Walkthroughs, tutorials, and user-facing documentation are updated to reflect the change. | [Doc Validation Protocol](/docs/governance/protocol/documentation-validation.md) |

## 2. Implementation Lifecycle

1. **Discovery**: Map the current state and identify debt.
2. **Planning**: Create an approved **Implementation Plan**.
3. **Execution**: Implement logic and concurrent documentation.
4. **Validation**: Execute automated scripts and manual walkthroughs.

## 3. Mandatory Artifacts

- **Implementation Plan**: Approved by a reviewer before execution.
- **Walkthrough**: Visual proof of work (screenshots/recordings).
- **Test Suite**: Automated verification for new behavior.
- **Signpost README**: Updated for any modified functional area.
