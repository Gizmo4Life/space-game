---
id: housekeeping-encapsulation
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Housekeeping Encapsulation

# Pattern: Housekeeping Encapsulation

UI panels and systems frequently perform multi-step setup operations — clearing a list, re-finding an entity, re-syncing local handles — at multiple points within the same class. When these operations are inlined at each callsite, any change to the logic requires updating every copy identically.

## Structure

Extract the repeated operation into a single private method on the class. Every callsite within that class invokes the method.

```cpp
// Encapsulated in a named method
void ShipyardPanel::refreshFleetList(const entt::registry& reg) {
  fleetEntities_ = getFleetEntities(reg); // delegates to UIUtils
}

// Called consistently at every change point
if (purchaseSucceeded) refreshFleetList(registry);
if (sellSucceeded)     refreshFleetList(registry);
```

## Forces

- The same operation is required at multiple callsites within one class.
- Inlining the operation at each site risks the copies diverging as the class evolves.
- The operation is specific enough to this class that it doesn't warrant a shared cross-class utility.

## Consequence

A single method body is the only place to update. Callsites become self-documenting — the method name describes intent, not implementation.

## Related

- [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) — Cross-class variant: when the same lookup is needed across multiple unrelated classes, it belongs in a shared utility rather than per-class methods.
- [single-source-calculation](/docs/developer/pattern/single-source-calculation.md) — Applies the same principle to derived value computation.
