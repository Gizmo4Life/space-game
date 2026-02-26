---
id: capability-elicitation-protocol
type: protocol
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Protocol](readme.md) > Capability Elicitation

## 1. Objective
Transform an initial premise or legacy code block into a detailed, bounded, and testable solution definition.

## 2. Modes of Operation

### Mode A: Greenfield (Premise-First)
1.  **Objective:** Execute [Elicitation Premise](/docs/developer/pattern/doc-elicitation-premise.md).
2.  **Refinement:** Execute [Elicitation Questioning](/docs/developer/pattern/doc-elicitation-questioning.md).
3.  **Constraint Identification:** Define technical and business limits.
4.  **Scope Refinement:** Execute [Elicitation Exclusivity](/docs/developer/pattern/doc-elicitation-exclusivity.md).


### Mode B: Brownfield (Code-First)
1. **Code Audit:** Analyze the legacy logic for patterns and behaviors.
2. **Reverse Intent:** Draft a "Problem Statement" based on what the code *appears* to be solving.
3. **Stakeholder Verification:** Confirm the reverse-engineered intent with the user.
4. **Logic Consolidation:** Identify redundant or "Ghost Logic" to be removed.

## 3. Exit Criteria
- **Action:** Create a [Context Elicitation Manifest](/docs/developer/pattern/doc-context-elicitation.md).
- **Verify:** The manifest contains measurable **Success Criteria**.
- **Verify:** The solution space is specifically bounded to direct [Test Planning](test-planning.md).
