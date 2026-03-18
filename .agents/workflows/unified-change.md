---
description: Combined Discovery and Greenfield protocol for standard-compliant implementation and testing.
---

# Workflow: Unified Change

This workflow automates the **[Unified Change Protocol](/docs/governance/protocol/unified-change.md)** by orchestrating two distinct agent roles.

## Collaboration Model

1. **[Primary Agent Elicitation](primary-agent-elicitation.md)**: The Primary Agent performs discovery, identifies success criteria, and creates an approved Implementation Plan.
2. **[Ralph Wiggum Loop](ralph-wiggum-loop.md)**: Sub-agents execute the approved plan iteratively until the **[Definition of Done](/docs/developer/pattern/definition-of-done.md)** is met.

## Instructions

1. **Consult Source Protocol:** Read [unified-change.md](/docs/governance/protocol/unified-change.md) to understand the governing rules for each phase.
2. **Execute Elicitation:** Follow the **[Primary Agent Elicitation](primary-agent-elicitation.md)** steps to define the scope and get user approval.
3. **Trigger Execution Loop:** Once the plan is approved, hand off the task to the **[Ralph Wiggum Loop](ralph-wiggum-loop.md)** for iterative implementation and validation.

> [!IMPORTANT]
> A change is only **DONE** when the Ralph Wiggum Loop has satisfied all criteria in the **[Definition of Done](/docs/developer/pattern/definition-of-done.md)** and the PR is submitted to `main`.
