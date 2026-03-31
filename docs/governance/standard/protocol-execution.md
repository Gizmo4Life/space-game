---
id: protocol-execution
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](readme.md) > Protocol Execution

## Context: Repository Modification & Automation
*Nuance: Protocols act as the agent's "Operating System" scripts. They must be strictly idempotent (safe to run multiple times) and explicitly separate state-analysis from state-mutation.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-gov-protocol](/docs/developer/pattern/doc-gov-protocol.md) | **P** | Required format for all execution scripts. |
| [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) | **P** | Every step must check existing state before mutating. |
| [logic-test-first](/docs/developer/pattern/logic-test-first.md) | **P** | Greenfield changes must define the test before implementation. |
| [agent-protocol-delegation](/docs/developer/pattern/agent-protocol-delegation.md) | **P** | Workflows delegate to canonical protocol docs; no inline logic. |
| [agent-iterative-compliance-loop](/docs/developer/pattern/agent-iterative-compliance-loop.md) | **P** | Gap-fix-validate loops bounded by a finite compliance checklist. |
| [agent-dual-role-orchestration](/docs/developer/pattern/agent-dual-role-orchestration.md) | **A** | Planner defines scope; executor iterates independently. |
| **Silent Overwrites** | **U** | Unacceptable. Discovery must flag outdated docs, not overwrite them without mapping the new pattern. |
| **Code Writing in Discovery** | **U** | Unacceptable. Discovery is a read-and-document operation only. |