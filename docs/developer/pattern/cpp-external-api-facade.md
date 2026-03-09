---
id: cpp-external-api-facade
type: pattern
tags: [architecture, isolation, refactoring]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > External API Facade

# Pattern: External API Facade

**Intent:** Isolate third-party library types behind a facade or wrapper to prevent external APIs from leaking unmanageably into game business logic.

## Shape

### 1. Ownership Boundaries
Only specific systems should directly communicate with or include third-party dependencies (like SFML or OpenTelemetry). 

```cpp
// BAD: Leaking External Types
// Game logic directly processes SFML events
void EconomyManager::handleInput(const sf::Event& event) { ... }

// GOOD: Facade Abstraction
// Game logic relies on an internal, game-specific input structure
void EconomyManager::handleInput(const GameInput& input) { ... }
```

### 2. Rendering Isolation
`RenderSystem` should ideally be the sole owner of "How to draw a shape." If UI panels return generic drawing commands or ECS components represent visual parameters, replacing the underlying rendering engine (or updating its target) becomes trivial.

## Key Constraints
- **Dependency Inversion:** The game logic defines the abstract interfaces it needs to operate. The external API wrappers implement those interfaces.
- **Header Hygiene:** Prevent external library headers (e.g., `<SFML/Graphics.hpp>`) from being included in core logic headers (`EconomyManager.h`).

## Applied In
- `RenderSystem`
