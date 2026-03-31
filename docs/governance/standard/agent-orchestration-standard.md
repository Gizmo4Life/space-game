---
id: agent-orchestration-standard
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Agent Orchestration

# Standard: Agent Orchestration (PADU)

This standard governs the fitness of patterns used to orchestrate AI agent workflows. It ensures that agent-driven modifications are traceable, bounded, and protocol-compliant.

## 1. Context: Workflow Structure & Delegation
*Nuance: Agent workflows automate multi-step repository operations. Embedding protocol logic directly in workflow files creates drift between the workflow and its governing protocol, leading to inconsistent behavior across agents.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [agent-protocol-delegation](/docs/developer/pattern/agent-protocol-delegation.md) | **P** | Preferred. Workflows are thin stubs that delegate to canonical protocol documents. |
| [agent-iterative-compliance-loop](/docs/developer/pattern/agent-iterative-compliance-loop.md) | **P** | Preferred. Execution bounded by a finite compliance checklist with build-test validation gates. |
| [agent-dual-role-orchestration](/docs/developer/pattern/agent-dual-role-orchestration.md) | **A** | Approved. Planner defines scope; executor iterates independently until done. |
| **inline-workflow-logic** | **D** | Deprecated. Embedding protocol steps directly in workflow files instead of delegating. |
| **unbounded-execution** | **U** | Unacceptable. Agent execution without a finite checklist or definition of done. |

## 2. Enforcement
- **Validation**: Every workflow file in `.agents/workflows/` MUST delegate to a protocol document via a file reference.
- **Review**: PRs that add procedural steps directly in a workflow file (instead of the governing protocol) will be rejected.
- **Audit**: During the Discovery protocol, any workflow containing more than 5 lines of procedural logic is a D-rated signal.
