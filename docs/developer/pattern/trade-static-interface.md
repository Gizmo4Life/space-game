---
id: trade-static-interface
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Static Trade Interface

# Pattern: Static Trade Interface

## 1. Geometry
- **Manager**: A global singleton or static class (e.g., `TradeManager`).
- **Methods**: Static functions that accept an `entt::registry`, `entt::entity` (buyer/seller), and a `ModuleId` or `ResourceId`.
- **Atomicity**: Transaction steps (check funds → debit → credit → log) must be contiguous and non-suspending.

## 2. Nuance
Used for cargo transactions and shipyard purchases where high-frequency state changes require a deterministic, non-ECS-system interface for user actions.

## 3. Verification
- [ ] Funds are debited before items are credited.
- [ ] Transaction fails gracefully if inventory is full or funds are insufficient.
- [ ] Every successful trade emits a `trade.transaction` telemetry span.
