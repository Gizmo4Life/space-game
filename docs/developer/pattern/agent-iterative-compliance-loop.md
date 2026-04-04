---
id: agent-iterative-compliance-loop
type: pattern
pillar: developer
category: agent
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Agent Iterative Compliance Loop

# Pattern: Agent Iterative Compliance Loop

An iterative refinement cycle that repeatedly identifies a gap against a compliance checklist, addresses it, validates the fix, and re-evaluates until the checklist is 100% satisfied.

## Structure

1. **Gap Identification**: Compare the current state against a finite requirements checklist.
2. **Address Gap**: Implement the minimum change to close the identified gap.
3. **Validate**: Run automated verification (build, test, lint).
4. **Re-evaluate**: Check remaining gaps. If any remain, return to step 1.
5. **Exit Condition**: All checklist items are satisfied.
