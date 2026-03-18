---
id: component-driven-initialization
type: pattern
pillar: developer
---

# Pattern: Component-Driven Initialization (for Tests)

## Context
In an ECS architecture, high-level summary components (like `ShipStats`) are often populated by synchronization systems (like `ShipOutfitter::refreshStats`) that read from primary source components (like `InstalledFuel`, `CargoComponent`, `InstalledModules`). 

## Preferred (P)
**Initialize the primary source components in your test setup.** This ensures that when the synchronization system runs, it correctly populates the summary fields, and your test operates on a state that is consistent with the actual game logic.

```cpp
// Preferred: Component-driven setup
auto& fuel = registry.emplace<InstalledFuel>(entity);
fuel.level = 100.0f; // Primary source
fuel.capacity = 100.0f;

// Sync summary fields from components
ShipOutfitter::instance().refreshStats(registry, entity, hull);
// Now stats.fuelStock is correctly set to 100.0f
```

## Unacceptable (U)
**Directly setting values in summary fields that are subject to overwriting.** This leads to tests that pass or fail based on the order of operations, or tests that break when a synchronization system is added or updated.

```cpp
// Unacceptable: Direct summary modification destined to be clobbered
auto& stats = registry.emplace<ShipStats>(entity);
stats.fuelStock = 100.0f; 

ShipOutfitter::instance().refreshStats(registry, entity, hull); 
// stats.fuelStock is NOW 0.0f because InstalledFuel component is missing!
```

## Nuance
If a system *only* reads from the summary field and no synchronization is triggered in the test, direct modification might work, but it is a "Discouraged (D)" pattern as it makes the test fragile to future architectural changes. Always aim for the primary source of truth.
