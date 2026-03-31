---
id: context-elicitation-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Context Elicitation

## Context: Semantic Discovery & Scope Control
*Nuance: Discovery must prevent "scope creep" by transforming vague intent into testable constraints. A failure to define boundaries is a failure of discovery.*

| requirement | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-elicitation-clarity](/docs/developer/pattern/doc-elicitation-clarity.md) | **P** | Verifiable success criteria. |
| [doc-elicitation-exclusivity](/docs/developer/pattern/doc-elicitation-exclusivity.md) | **P** | Explicit "Out of Scope" boundaries. |
| [doc-elicitation-traceability](/docs/developer/pattern/doc-elicitation-traceability.md) | **P** | 1:1 mapping to Test Plans. |
| [doc-context-elicitation](/docs/developer/pattern/doc-context-elicitation.md) | **P** | Discovery phase defining scope boundaries. |
| [doc-elicitation-premise](/docs/developer/pattern/doc-elicitation-premise.md) | **P** | Core functional justification before implementation. |
| [doc-elicitation-questioning](/docs/developer/pattern/doc-elicitation-questioning.md) | **P** | Agent inquiry for unresolved ambiguities. |
| [definition-of-done](/docs/developer/pattern/definition-of-done.md) | **P** | Binary gating criteria for feature completion. |
| [padu-evaluation](/docs/developer/pattern/padu-evaluation.md) | **P** | Strict 4-tier fitness rating for structural shapes. |
| [test-case-duality](/docs/developer/pattern/test-case-duality.md) | **P** | Business requirements mapped 1:1 to automated test cases. |
| **Ambiguity** | **U** | Subjective terms without metrics. |
| **Infinite Scope** | **U** | Lack of a hard stop on features. |

## Context: External Contracts & Agent Boundaries
*Nuance: Defining how the AI agent interacts with the user and external system state.*

| requirement | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-ext-contract](/docs/developer/pattern/doc-ext-contract.md) | **P** | Hard boundary definition between agent responsibilities and user. |
| [doc-ext-integration](/docs/developer/pattern/doc-ext-integration.md) | **P** | Strategy for merging agent work into existing codebases. |
| [ext-brownfield-context](/docs/developer/pattern/ext-brownfield-context.md) | **P** | Adapting to legacy boundaries. |
| [ext-greenfield-context](/docs/developer/pattern/ext-greenfield-context.md) | **P** | Establishing new boundaries. |
| **Implicit Contracts** | **U** | Assuming user intent without validation. |

