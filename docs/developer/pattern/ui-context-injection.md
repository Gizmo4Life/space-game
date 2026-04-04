---
id: ui-context-injection
type: pattern
polarity: prescriptive
pillar: developer
category: ux
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: UI Context Injection

# Pattern: UI Context Injection

**Intent:** Provide UI components with the necessary game state (entities, registries) via a structured context object rather than allowing components to perform broad queries on the registry.

## Shape

Instead of:
```cpp
void MyPanel::render(entt::registry& registry) {
    auto view = registry.view<PlayerComponent>();
    // ... search for flagship ...
}
```

Use:
```cpp
struct UIContext {
    entt::registry& registry;
    entt::entity player;
    entt::entity target;
};

void MyPanel::render(const UIContext& ctx) {
    auto& playerComp = ctx.registry.get<PlayerComponent>(ctx.player);
    // ... direct access ...
}
```

## Key Constraints
- **Responsibility**: The `UIFramework` or `RenderSystem` is responsible for resolving the current "active" entities once per frame.
- **Immutability**: The UI should generally treat the context as read-only during the `render()` pass.
- **Simplicity**: Avoid deep-nesting context objects.

## Benefits
- **Performance**: Reduces O(N) registry searches to O(1) direct access.
- **Predictability**: Ensures the UI is always using a consistent "frame of reference" for the player and target.
- **Testability**: Makes it easier to mock the game state for UI unit tests.
