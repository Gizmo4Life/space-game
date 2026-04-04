---
id: cpp-component-aggregation
type: pattern
tags: [cpp, ecs, logic]
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > C++ Component Aggregation

## Intent
Prevent logic duplication and data inconsistency by centralizing the derivation of complex state from multiple ECS components.

## Motivation
In the *Space Game*, ship stats like total power or volume are distributed across multiple components (`InstalledEngines`, `InstalledWeapons`, `InstalledPower`, etc.). Calculating these values manually in UI panels leads to "iteration breaks" where adding a new module type requires updating every UI file.

## Rules
1. **Manager Centralization**: Derived statistics that rely on multiple components should be calculated in a dedicated Manager class (e.g., `ShipOutfitter`) or a specific "Stats" component (`ShipStats`).
2. **Refresh Systems**: Implement "Refresh" methods or systems that update an aggregate component whenever its dependencies change, rather than re-calculating on every frame.
3. **UI Agnosticism**: UI panels should only read from aggregate data or call a one-line helper. They should never iterate over raw component lists to sum values if those values exist elsewhere.

## Examples

### Bad (Manual Aggregation in UI)
```cpp
// ShipyardPanel.cpp
float totalPower = 0;
totalPower += registry.get<InstalledPower>(entity).output;
totalPower -= registry.get<InstalledEngines>(entity).draw;
// Missing Weapons! This is a bug waiting to happen.
```

### Good (Aggregated via Manager/Component)
```cpp
// Logic side
void ShipOutfitter::refreshStats(::entt::registry &reg, ::entt::entity e) {
    auto &stats = reg.get_or_emplace<ShipStats>(e);
    stats.powerBalance = calculateTotalPower(reg, e);
}

// UI side
const auto &stats = registry.get<ShipStats>(entity);
drawText("Power: " + fmt(stats.powerBalance));
```
