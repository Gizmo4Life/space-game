---
id: protocol-execution
type: standard
pillar: governance
---
[Home](/) > [Governance] > [Standard] > Protocol Execution

## Context: Repository Modification & Automation
*Nuance: Protocols act as the agent's "Operating System" scripts. They must be strictly idempotent (safe to run multiple times) and explicitly separate state-analysis from state-mutation.*

| Pattern | Rating | Contextual Nuance |
| :--- | :--- | :--- |
| [doc-gov-protocol](/docs/developer/pattern/doc-gov-protocol.md) | **P** | Required format for all execution scripts. |
| **Idempotent Actions** | **P** | Every step must check existing state before mutating (e.g., "If file missing, create..."). |
| **Test-First Verification** | **P** | Greenfield changes must define the test/verification before writing implementation. |
| **Silent Overwrites** | **U** | Unacceptable. Discovery must flag outdated docs, not overwrite them without mapping the new pattern. |
| **Code Writing in Discovery** | **U** | Unacceptable. Discovery is a read-and-document operation only. |