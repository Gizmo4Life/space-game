---
id: developer-pattern-manifest
type: manifest
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > Pattern

# Sub-pillar: Pattern

Atomic structural definitions for code and documents.

## CI/CD & Infrastructure
*Nuance: Patterns governing build isolation, security, and automated testing layers.*
- [cicd-immutable-artifact](cicd-immutable-artifact.md)
- [cicd-isolated-build](cicd-isolated-build.md)
- [cicd-prioritized-testing](cicd-prioritized-testing.md)
- [cicd-reproducible-build](cicd-reproducible-build.md)
- [cicd-secret-vaulting](cicd-secret-vaulting.md)
- [cicd-test-layering](cicd-test-layering.md)
- [cicd-vulnerability-scan](cicd-vulnerability-scan.md)

## Architectural Geometry (T1-T3)
*Nuance: Defining the hierarchical levels of the repository (Landscape, Capability, Module).*
- [doc-t1-landscape](doc-t1-landscape.md)
- [doc-t2-capability](doc-t2-capability.md)
- [doc-t2-with-code](doc-t2-with-code.md)
- [doc-t3-module](doc-t3-module.md)
- [doc-t3-with-biz-logic](doc-t3-with-biz-logic.md)
- [doc-pillar-ownership](doc-pillar-ownership.md)
- [doc-module-dependency](doc-module-dependency.md)

## Documentation UI/UX
*Nuance: Patterns for ensuring machine-readability and human scannability.*
- [doc-breadcrumb-navigation](doc-breadcrumb-navigation.md)
- [doc-flat-hierarchy](doc-flat-hierarchy.md)
- [doc-yaml-metadata](doc-yaml-metadata.md)
- [doc-structured-readme](doc-structured-readme.md)
- [signpost-readme](signpost-readme.md)
- [doc-walkthrough](doc-walkthrough.md)

## Operational & Incident Response
*Nuance: High-density artifacts for system uptime and restoration.*
- [doc-ops-alert](doc-ops-alert.md)
- [doc-ops-span-runbook](doc-ops-span-runbook.md)
- [doc-ops-unified-runbook](doc-ops-unified-runbook.md)
- [doc-ops-restoration-step](doc-ops-restoration-step.md)
- [ops-triage-path](ops-triage-path.md)
- [ops-escalation-path](ops-escalation-path.md)

## Elicitation & Requirements
*Nuance: Shapes for transforming vague intent into testable constraints.*
- [doc-context-elicitation](doc-context-elicitation.md)
- [doc-elicitation-clarity](doc-elicitation-clarity.md)
- [doc-elicitation-exclusivity](doc-elicitation-exclusivity.md)
- [doc-elicitation-traceability](doc-elicitation-traceability.md)
- [doc-elicitation-premise](doc-elicitation-premise.md)
- [doc-elicitation-questioning](doc-elicitation-questioning.md)

## External Integration
*Nuance: Contexts for repository consumption by 3rd parties or legacy projects.*
- [doc-ext-contract](doc-ext-contract.md)
- [doc-ext-integration](doc-ext-integration.md)
- [ext-greenfield-context](ext-greenfield-context.md)
- [ext-brownfield-context](ext-brownfield-context.md)

## Logic, Testing & Verification
*Nuance: Rules for code correctness and automated verification.*
- [logic-idempotency](logic-idempotency.md)
- [logic-test-first](logic-test-first.md)
- [test-case-duality](test-case-duality.md)
- [padu-evaluation](padu-evaluation.md)

## Governance & Meta
*Nuance: Patterns governing the creation of other documentation artifacts.*
- [doc-gov-protocol](doc-gov-protocol.md)
- [doc-gov-standard](doc-gov-standard.md)
- [doc-dichotomy](doc-dichotomy.md)

## Anti-Patterns (Discouraged/Unacceptable)
*Nuance: Forbidden or legacy shapes that trigger audit failures.*
- [doc-directory-nesting](doc-directory-nesting.md)
- [doc-monolithic-wiki](doc-monolithic-wiki.md)
- [doc-narrative-paragraphs](doc-narrative-paragraphs.md)
- [doc-ops-monolithic-runbook](doc-ops-monolithic-runbook.md)
- [doc-ops-unverified-mitigation](doc-ops-unverified-mitigation.md)
- [readme-long-prose](readme-long-prose.md)
