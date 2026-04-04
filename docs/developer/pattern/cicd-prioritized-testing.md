---
id: cicd-prioritized-testing
type: pattern
tags: [foundation, cicd]
category: cicd
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Prioritized Testing

# Pattern: Prioritized Testing

Staging automated tests based on execution speed and failure likelihood to provide the fastest possible feedback loop.

## Why it's Required
- **Developer Velocity:** Catch linting and unit errors in seconds rather than waiting for 20-minute E2E suites.
- **Resource Efficiency:** Avoid running expensive integration tests if basic logic is broken.

## Stages
1. **Static Analysis:** Linting, Formatting, Type-checking.
2. **Unit Tests:** Atomic logic checks (mocked).
3. **Integration/E2E:** Full-stack flows (real/simulated DB).
