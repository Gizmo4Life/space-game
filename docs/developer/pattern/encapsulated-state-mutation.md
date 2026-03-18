---
id: encapsulated-state-mutation
type: pattern
pillar: developer
---

# Pattern: Encapsulated State Mutation

## Context
Components often manage complex internal state with interconnected dependencies (e.g., `CargoComponent` tracks both the inventory map and the `currentWeight` total). Direct access to internal data structures bypasses the logic required to keep the component synchronized, leading to data corruption and logic errors.

## Preferred (P)
**Use dedicated API methods for all state changes.** This ensures that any side-effects (like recalculating mass, updating telemetry, or triggering events) are handled automatically.

```cpp
// Preferred: Encapsulated mutation 
auto& cargo = registry.get<CargoComponent>(entity);
cargo.add(Resource::Food, 100.0f); // Updates currentWeight and triggers side-effects
```

## Discouraged (D)
**Directly modifying internal maps or vectors.** This is a common source of bugs where a value is updated but its dependent stats (like `currentWeight`) are not.

```cpp
// Discouraged: Direct access bypassing side-effects
auto& cargo = registry.get<CargoComponent>(entity);
cargo.inventory[Resource::Food] += 100.0f; // Weight is NO LONGER SYNCED!
```

## Nuance
Even in unit tests, it is better to use the high-level API. If a test bypasses the API to setup state, it is testing a "broken" object that doesn't represent how the actual game operates.
