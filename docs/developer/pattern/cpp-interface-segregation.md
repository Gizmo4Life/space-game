---
id: cpp-interface-segregation
type: pattern
tags: [architecture, stability, refactoring]
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Interface Segregation

# Pattern: Interface Segregation

**Intent:** Prevent structural brittleness by always requiring the *least privileged* or *most generic* interface that satisfies the needs of a function, avoiding concrete coupling.

## Shape

### 1. Abstract Parameters Over Concrete Implementations
Functions should depend on base classes or interfaces rather than specific implementations whenever generic functionality is required. 

```cpp
// BAD: Concrete Coupling
// If we switch to off-screen rendering (RenderTexture), every function
// taking RenderWindow breaks.
void drawShip(sf::RenderWindow& window, const Ship& ship);

// GOOD: Interface Segregation
// Passing the base RenderTarget allows drawing to windows or textures safely.
void drawShip(sf::RenderTarget& target, const Ship& ship);
```

### 2. Isolate Scope Requirements
If a function only needs to read a single property or perform a narrow action, pass only what is strictly required rather than the entire god-object or system state. 

## Key Constraints
- **Minimal Privilege:** Only provide the dependencies necessary to complete the operation. 
- **Refactoring Resilience:** Concrete coupling forces cascading modifications across translation units when a base implementation needs to change.

## Applied In
- `RenderSystem` (e.g. `sf::RenderTarget`)
- All UI Panels (`InfoPanel`, `MarketPanel`)
