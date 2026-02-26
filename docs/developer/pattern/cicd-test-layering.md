---
id: cicd-test-layering
type: pattern
tags: [cicd, testing, quality]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > CICD Test Layering

## Structure
- **Layer 1: Unit Tests:** Rapid verification of atomic logic. Must pass before proceeding.
- **Layer 2: Integration Tests:** Verification of module interactions and side-effects.
- **Layer 3: E2E/System Tests:** Verification of critical business paths in a production-like environment.
- **Verify:** Critical path coverage is maintained on every PR.
