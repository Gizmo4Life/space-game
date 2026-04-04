---
id: agent-protocol-delegation
type: pattern
pillar: developer
category: agent
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Agent Protocol Delegation

# Pattern: Agent Protocol Delegation

A workflow file acts as a thin routing stub that delegates all behavioral logic to a canonical protocol document. The workflow contains no domain logic — only a pointer to the source of truth.

## Structure

1. A **workflow file** defines the entry point for an automated action.
2. The workflow's only instruction is to read and execute a **protocol document** at a specified path.
3. The protocol document contains the full procedural steps.
4. Updates to the process are made exclusively in the protocol; the workflow stub remains stable.
