---
id: centralized-entity-lookup
type: pattern
pillar: developer
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Centralized Entity Lookup

# Pattern: Centralized Entity Lookup

When multiple unrelated classes need to locate the same canonical entity (e.g., the player's flagship, the active planet), each class independently scanning the ECS registry introduces divergent implementation details — different components checked, different fallback behavior — that can produce inconsistent results as the codebase evolves.

## Structure

Move the lookup into a single shared utility function. All callsites delegate to that function instead of implementing their own scan.

```cpp
// UIUtils.h — canonical authority
namespace space {
  entt::entity findFlagship(const entt::registry& registry);
  ShipBlueprint blueprintFromEntity(const entt::registry& registry,
                                   entt::entity entity);
}

// Any panel or system — no inline loop
entt::entity player = findFlagship(registry);
if (!registry.valid(player)) return;
```

## Forces

- The canonical entity is globally unique but may change identity (e.g., flagship swap on ship purchase).
- Many systems need to locate it independently, at different points in the frame.
- The identification criteria may become more complex over time (e.g., checking `isFlagship` flag vs. a tag component).

## Consequence

All callsites automatically benefit when the lookup logic is updated. A single test suite for the utility covers all consumers.

## Related

- [housekeeping-encapsulation](/docs/developer/pattern/housekeeping-encapsulation.md) — Intra-class variant of the same principle.
- [single-source-calculation](/docs/developer/pattern/single-source-calculation.md) — Applies the same principle to derived calculations.
