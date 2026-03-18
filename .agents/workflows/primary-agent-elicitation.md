---
description: Primary Agent workflow for success criteria elicitation and change planning.
---

# Workflow: Primary Agent Elicitation

This workflow is used by the **Primary Agent** to identify success criteria, decide on the next action, and create an approved **[Implementation Plan](/docs/developer/pattern/definition-of-done.md#3-mandatory-artifacts)**.

## 1. Introspection & Discovery
1. Perform a **[Discovery Protocol](/docs/governance/protocol/discovery.md)** audit of the target modules.
2. Identify all technical debt, documentation drift, and standard violations.
3. Consult the **[Definition of Done](/docs/developer/pattern/definition-of-done.md)** to identify mandatory output requirements.

## 2. Success Criteria Elicitation
1. Define the **Success Criteria** for the requested change. 
   - *Requirement:* Criteria must be concrete and testable.
   - *Requirement:* Must include functional and architectural constraints.
2. Formulate the "Next Action" strategy based on the elicitated requirements.

## 3. Implementation Planning
1. Create an **[Implementation Plan](/docs/developer/pattern/definition-of-done.md#3-mandatory-artifacts)** that satisfies all success criteria and DoD goals.
2. Include a **Verification Plan** with specific unit tests and telemetry probes.
3. Get explicit **User Approval** for the plan before proceeding.

## 4. Work Handoff
1. Once the plan is approved, trigger the **[Ralph Wiggum Loop Workflow](ralph-wiggum-loop.md)** to execute the change.
2. Provide the sub-agents with the approved plan and success criteria.
3. Monitor the loop for any design-blocking questions.

> [!TIP]
> The Primary Agent's goal is to ensure the **Plan** is robust enough that sub-agents can execute the **Ralph Wiggum Loop** without further design input.
