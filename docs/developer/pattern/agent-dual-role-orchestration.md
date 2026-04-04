---
id: agent-dual-role-orchestration
type: pattern
pillar: developer
category: agent
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Agent Dual-Role Orchestration

# Pattern: Agent Dual-Role Orchestration

A two-role orchestration model where a **planning role** defines scope and success criteria, then hands off execution to an **executor role** that iterates independently until a compliance gate is satisfied.

## Structure

1. **Planner Role**: Performs discovery, elicits success criteria, and produces an approved plan.
2. **Handoff**: The plan and criteria are passed to the executor as immutable inputs.
3. **Executor Role**: Iterates through an [Agent Iterative Compliance Loop](agent-iterative-compliance-loop.md) to implement the plan.
4. **Escalation Path**: If the executor encounters a design-blocking question, control returns to the planner.
