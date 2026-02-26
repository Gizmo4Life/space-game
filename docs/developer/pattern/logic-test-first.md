---
id: logic-test-first
type: pattern
tags: [logic, verification, greenfield]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Test-First Verification

# Pattern: Test-First Verification

To ensure requirements are met, verification criteria must be defined before the implementation of any greenfield changes.

## Structure
- **Verification Definition:** Explicitly state the CLI command, query, or manual check that will prove the task is complete.
- **Failure Mode:** Define what an unsuccessful execution looks like.
- **Implementation:** Proceed with code/document changes ONLY after the verification protocol is locked.
