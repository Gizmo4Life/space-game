---
id: single-source-calculation
type: pattern
pillar: developer
---

# Pattern: Single Source of Calculation

## Context
When multiple systems depend on a derived value (e.g., ship mass, total thrust), implementing the calculation logic in each system leads to redundancy and, more importantly, inconsistency as systems evolve at different rates.

## Preferred (P)
**Perform the calculation in exactly one system or utility.** Store the result in a shared component for other systems to consume.

```cpp
// ShipOutfitter.cpp (Source of Truth for Dry Mass)
void ShipOutfitter::refreshStats(entt::registry& registry, entt::entity entity) {
  float dryMass = hull.baseMass;
  for (const auto& m : modules) dryMass += m.mass;
  stats.dryMass = dryMass; // Calculated once
}

// KinematicsSystem.cpp (Consumer)
void KinematicsSystem::update(entt::registry& registry) {
  // Use the dryMass calculated by ShipOutfitter
  stats.wetMass = stats.dryMass + currentResourceWeight;
}
```

## Unacceptable (U)
**Redefining the calculation logic in a second system.** This creates a risk that the second system will miss updates (like a new module type that adds mass) or use different base assumptions (like ignoring module mass entirely).

```cpp
// KinematicsSystem.cpp (Redundant/Conflicting logic)
void KinematicsSystem::update(entt::registry& registry) {
  // WRONG: Ignores modules and conflicts with ShipOutfitter
  stats.dryMass = hull.baseMass * hull.massMultiplier; 
  stats.wetMass = stats.dryMass + currentResourceWeight;
}
```

## Nuance
If performance is a concern, use a `dirty` flag to trigger the single-source calculation only when inputs change. Do NOT opt for local recalculation as a performance optimization unless you are prepared to maintain parity across all sites.
