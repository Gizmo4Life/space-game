---
id: housekeeping-encapsulation
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Housekeeping Encapsulation

# Pattern: Housekeeping Encapsulation

## Context
UI Panels and Systems often require "housekeeping" tasks (e.g., finding the player's flagship, refreshing a list of entities, re-syncing local handles after a state change). Inlining these multi-step operations leads to code duplication and increases the risk of subtle bugs if the logic is updated in one place but not another.

## Preferred (P)
**Move repetitive setup and lookup logic into named helper methods.** This improves readability and provides a single point of update for common operations.

```cpp
// Preferred: Encapsulated helper
void ShipyardPanel::refreshFleetList(const registry& reg, entity player) {
  fleetEntities_.clear();
  // ... standardized lookup logic ...
}

// Usage in handleEvent
if (success) refreshFleetList(registry, player);
```

## Undesired (D)
**Inlining lookup loops or list population multiple times within the same class.**

```cpp
// Undesired: Redundant inline logic
if (mode == Sell) {
  fleetEntities_.clear();
  // ... loop 1 ...
}
// ... 100 lines later ...
if (success) {
  fleetEntities_.clear();
  // ... loop 2 (identical to loop 1) ...
}
```

## Unacceptable (U)
**Hardcoding specific entity IDs or bypassing standard registry views for common lookups.**

## Nuance
If a housekeeping task is needed across multiple classes, consider moving it to a `UIUtils` or a static helper in the relevant Component (e.g., `PlayerComponent::getFlagship(registry)`).
