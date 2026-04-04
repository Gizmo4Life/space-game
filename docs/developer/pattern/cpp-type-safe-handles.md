---
id: cpp-type-safe-handles
type: pattern
tags: [safety, types, refactoring]
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Type-Safe Handles

# Pattern: Type-Safe Handles

**Intent:** Prevent accidental identity mixing and enforce compiler correctness by using strong typedefs or explicit handle structures instead of raw pointers (`void*`), `int` IDs, or opaque indices.

## Shape

### 1. Strong Typing for Identifiers
If a system manages resources via an ID, wrap that ID in a struct or clearly defined strongly-typed handle rather than passing around `size_t` or `int`.

```cpp
// BAD: Generic IDs
// The compiler cannot prevent you from passing a ShipID where a PlanetID is expected.
void dockAtPlanet(int shipId, int planetId);

// GOOD: Type-Safe Handles
// b2WorldId is a strong typedef in Box2D v3. 
// You cannot accidentally mix it with a b2BodyId.
void applyPhysics(b2WorldId world, b2BodyId body);
```

### 2. Eliminating Raw Standard Collections for Entities
Do not pass `std::vector<int>` when dealing with domain entities. If necessary, use domain-specific structures or ECS abstractions (`entt::entity`) that explicitly denote their purpose. 

## Key Constraints
- **Zero-Cost Abstractions:** In modern C++, wrapping a primitive in a struct has zero runtime overhead but massive compile-time benefits.
- **Self-Documenting Signatures:** Functions that take `FactionID` and `HullID` are immediately self-documenting compared to functions that take `int` and `int`.

## Applied In
- `Box2D 3.x Integration` (e.g. `b2WorldId`, `b2BodyId`)
- `EnTT` (e.g. `entt::entity`)
