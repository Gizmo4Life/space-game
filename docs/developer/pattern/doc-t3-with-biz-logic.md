---
id: doc-t3-with-biz-logic
type: pattern
tags: [anti-pattern, architecture]
category: geometry
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > T3 with Business Logic

# Anti-pattern: T3 detailing Business Logic

Embedding high-level business rules or orchestration logic into low-level implementation mapping (T3).

## Why it's Forbidden
- **Violates Separation of Concerns:** T3 should be a "dumb" map between patterns and files. 
- **Drift Risk:** Business logic changes more frequently than code structure. Mixing them complicates maintenance.

## Corrective Action
Extract business logic to a [T2 Capability](/docs/developer/pattern/doc-t2-capability.md).
