---
id: cicd-pipeline
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > CI/CD Pipeline

# Standard: CI/CD Pipeline

This standard defines the requirements and best practices for Continuous Integration (CI) and Continuous Deployment (CD) pipelines within this repository.

## 1. Build Isolation & Reproducibility
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cicd-isolated-build](/docs/developer/pattern/cicd-isolated-build.md) | **P** | Prevents host state contamination. | N/A |
| [cicd-reproducible-build](/docs/developer/pattern/cicd-reproducible-build.md) | **P** | Deterministic outputs from identical inputs. | N/A |
| [cicd-shadow-build](/docs/developer/pattern/cicd-shadow-build.md) | **A** | Out-of-tree builds for permission resilience. | Acceptable for sandbox environments where root directory mutation is restricted and build artifacts must be siloed. |
| **Host-dependent build** | **U** | Unacceptable. Relying on global binaries. |
## 2. Automated Testing Strategy
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cicd-prioritized-testing](/docs/developer/pattern/cicd-prioritized-testing.md) | **P** | Fail fast by running fast tests first. | N/A |
| [cicd-test-layering](/docs/developer/pattern/cicd-test-layering.md) | **P** | Distinct boundaries between unit, integration, and E2E. | N/A |
| **Monolithic slow tests** | **D** | Unacceptable. Flaky or sluggish tests block CI. |
## 3. Quality Gates & Promotion
- **Check:** Every Pull Request must pass the automated CI suite before it can be merged.
- **Manual Gate:** Deployment to production requires a successful [PR Review](/docs/governance/protocol/pull-request-review.md) and manual approval.
- **Integrity:** The [Documentation Validation](/docs/governance/protocol/documentation-validation.md) protocol must pass as part of the pipeline.

## 4. Artifact Management
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cicd-immutable-artifact](/docs/developer/pattern/cicd-immutable-artifact.md) | **P** | Build once, deploy across environments. | N/A |
| **Traceability** | **P** | Every deployment linked to build ID. |
| **Recompiling per environment** | **U** | Unacceptable. Introduces drift. |

## 5. Security & Compliance
| Pattern | Rating | Contextual Nuance | Acceptable Context |
| :--- | :--- | :--- | :--- |
| [cicd-secret-vaulting](/docs/developer/pattern/cicd-secret-vaulting.md) | **P** | Ephemeral, injected secrets. | N/A |
| [cicd-vulnerability-scan](/docs/developer/pattern/cicd-vulnerability-scan.md) | **P** | Automated container/dep scanning. | N/A |
| **Hardcoded credentials** | **U** | Immediate termination offense. |
