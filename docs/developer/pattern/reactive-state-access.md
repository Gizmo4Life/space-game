---
id: reactive-state-access
type: pattern
pillar: developer
---

# Pattern: Reactive State Access

## Context
"Mirroring" state from one component (e.g., `CargoComponent`) into a summary component (e.g., `ShipStats`) creating a synchronization burden. Every time the source changes, the summary must be updated. Forgetting to update the summary leads to "stale" data and logic failures.

## Preferred (P)
**Systems should query the "Source of Truth" directly or use a reactive update.** Avoid manual mirroring where possible. If a value is needed for display (HUD), the UI should query the source component (e.g., `CargoComponent`) instead of a mirrored field in `ShipStats`.

```cpp
// Preferred: Direct access in system
void KinematicsSystem::update(entt::registry& registry) {
  if (auto* cargo = registry.try_get<CargoComponent>(entity)) {
    float wetMass = stats.dryMass + cargo->currentWeight; // Use the direct source
    b2Body_SetMass(bodyId, wetMass);
  }
}
```

## Undesired (D)
**Manual mirroring or "cached" fields that are not automatically synchronized.** This is a common source of bugs where the HUD shows 0 food while the inventory has 100.

```cpp
// Undesired: Mirroring state
stats.foodStock = cargo->inventory[Resource::Food]; // Manual sync required everywhere food changes
```

## Unacceptable (U)
**Modifying source data while bypassing its internal synchronization.** This breaks the consistency of the source component itself.

```cpp
// Unacceptable: Bypassing CargoComponent::currentWeight logic
cargo->inventory[Resource::Food] -= 10.0f; // currentWeight is out of sync!
```

## UI Example
UI panels should query derived statistics from a specialized stats component (e.g., `ShipStats`) rather than attempting to recalculate them from raw components (e.g., `CargoComponent`, `InstalledModules`).

*   **Undesired**: Reading `CargoComponent` manually in `ShipyardPanel` to estimate weight.
*   **Preferred**: Reading `ShipStats::wetMass` which is already synchronized by the engine.

This reduces the complexity of UI code and ensures that any balance changes to the mass calculation are automatically reflected in the interface.
