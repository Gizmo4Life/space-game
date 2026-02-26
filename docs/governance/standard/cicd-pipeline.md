---
id: cicd-pipeline
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > CI/CD Pipeline

# Standard: CI/CD Pipeline

This standard defines the requirements and best practices for Continuous Integration (CI) and Continuous Deployment (CD) pipelines within this repository.

## 1. Build Isolation & Reproducibility
- **Requirement:** Adhere to [Isolated Build](/docs/developer/pattern/cicd-isolated-build.md) and [Reproducible Build](/docs/developer/pattern/cicd-reproducible-build.md) patterns.


## 2. Automated Testing Strategy
- **Prioritization:** Adhere to [Prioritized Testing](/docs/developer/pattern/cicd-prioritized-testing.md) and [Test Layering](/docs/developer/pattern/cicd-test-layering.md).


## 3. Quality Gates & Promotion
- **Check:** Every Pull Request must pass the automated CI suite before it can be merged.
- **Manual Gate:** Deployment to production requires a successful [PR Review](/docs/governance/protocol/pull-request-review.md) and manual approval.
- **Integrity:** The [Documentation Validation](/docs/governance/protocol/documentation-validation.md) protocol must pass as part of the pipeline.

## 4. Artifact Management
- **Versioning:** Adhere to [Immutable Artifact](/docs/developer/pattern/cicd-immutable-artifact.md).
- **Traceability:** Every deployment linked to build ID.

## 5. Security & Compliance
- **Vaulting:** Adhere to [Secret Vaulting](/docs/developer/pattern/cicd-secret-vaulting.md).
- **Scanning:** Adhere to [Vulnerability Scan](/docs/developer/pattern/cicd-vulnerability-scan.md).

