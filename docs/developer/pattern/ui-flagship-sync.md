---
id: ui-flagship-sync
type: pattern
pillar: developer
category: logic
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: UI Flagship Synchronization

# Pattern: UI Flagship Synchronization

The player flagship is a volatile entity that can be destroyed, sold, or transferred during gameplay. UI panels that store or receive a flagship entity handle MUST ensure they are not using a stale reference, especially after an event that could mutate the player's fleet.

## 1. Problematic Pattern (Avoid)
Storing an `entt::entity` handle and assuming it remains valid for the duration of an event handler or across multiple frames without re-verification.

```cpp
// AVOID: Stale reference usage
void handleEvent(const sf::Event& event, const UIContext& ctx) {
    // ctx.player might have been destroyed by a previous event handler this frame
    auto& credits = registry.get<CreditsComponent>(ctx.player); // CRASH if ctx.player is invalid
}
```

## 2. Preferred Pattern (Acceptable)
Always verify entity validity and component presence before access. If an action changes the flagship, refresh the reference immediately using `findFlagship(registry)`.

```cpp
// PREFERRED: Defensive resolution
void handleEvent(const sf::Event& event, const UIContext& ctx) {
    auto currentFlagship = registry.valid(ctx.player) ? ctx.player : findFlagship(registry);
    
    if (registry.valid(currentFlagship) && registry.all_of<CreditsComponent>(currentFlagship)) {
        auto& credits = registry.get<CreditsComponent>(currentFlagship);
        // ... action ...
    }
}
```

## 3. Propagation Protocol
When a UI panel changes the player's flagship (e.g., via `buyShip` or `sellShip`), the new flagship ID must be propagated back to the parent screen and the main simulation loop to ensure full system synchronization.

1. **Local Update**: The panel calls `findFlagship(registry)` to update its local view.
2. **Intermediate Update**: The `LandingScreen` refreshes its `playerEntity_` member.
3. **Simulation Update**: `main.cpp` refreshes its `playerEntity` variable after the UI processing block.
