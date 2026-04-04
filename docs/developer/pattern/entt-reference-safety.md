---
id: entt-reference-safety
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: EnTT Reference Safety (Synchronization Hubs)

# Pattern: EnTT Reference Safety (Synchronization Hubs)

## Context
In the EnTT ECS, calling `registry.replace<T>(entity, ...)` or similar operations can invalidate existing references or pointers to components of type `T`. Synchronization hubs like `ShipOutfitter::refreshStats` frequently update multiple components, making it dangerous to hold references across these calls.

## Preferred (P)
**Always re-fetch component references after calling any function that might modify the registry.** Do not store component pointers or references for long periods, especially across sync boundaries.

```cpp
// Preferred: Re-fetch after sync
auto &stats_initial = registry.get<ShipStats>(entity);
// ... do something ...

ShipOutfitter::instance().refreshStats(registry, entity, hull);

// RE-FETCH: The old stats_initial reference might be dangling or stale
auto &stats = registry.get<ShipStats>(entity); 
REQUIRE(stats.wetMass > 0);
```

## Unacceptable (U)
**Holding and using component references across synchronization calls.**

```cpp
// Unacceptable: Using a potentially invalidated reference
auto &stats = registry.get<ShipStats>(entity);

ShipOutfitter::instance().refreshStats(registry, entity, hull);

// CRASH or STALE DATA: stats might point to old memory 
// if ShipOutfitter used registry.replace or reallocated the pool
float mass = stats.wetMass; 
```

## Nuance
This is particularly critical in unit tests where `refreshStats` is called to prepare the state. Always treat "Sync" functions as memory-invalidation boundaries for component references.
