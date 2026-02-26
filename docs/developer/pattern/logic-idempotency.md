---
id: logic-idempotency
type: pattern
tags: [logic, automation, protocol]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Logic Idempotency

# Pattern: Idempotent Actions

Automation logic must be safe to execute multiple times without unintended side effects or duplicate state.

## Structure
- **Check-First:** Every mutation step must be preceded by an existence or state check.
- **Conditional Execution:**
  - "If file `x` exists, do nothing."
  - "If string `y` is present in file `z`, do not append."
- **Expected Outcome:** Re-running the logic results in the same final state regardless of the initial starting conditions.
