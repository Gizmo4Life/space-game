---
id: trade-tabbed-interface
type: pattern
pillar: developer
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Tabbed Trade Interface

# Pattern: Tabbed Trade Interface

## Context
The landing screen on planets provides multiple services: information, shipyard, outfitter, and commodity market. A unified, clear navigation system is required to switch between these contexts without cluttering the UI.

## Shape
The interface uses a **Tabbed Context Switcher** pattern.

### Components
1.  **State Manager**: Tracks the active tab index (`activeTab`).
2.  **Navigation Handler**: Map numeric keys (1-4) to screen contexts.
3.  **Contextual Renderers**: Individual functions or classes that render the specific content for the active tab.

### Navigation Logic
```cpp
if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Num1) activeTab = PlanetInfo;
    if (event.key.code == sf::Keyboard::Num2) activeTab = Shipyard;
    if (event.key.code == sf::Keyboard::Num3) activeTab = Outfitter;
    if (event.key.code == sf::Keyboard::Num4) activeTab = Market;
}
```

### Visual Feedback
The active tab is highlighted in the header, and the main viewport is cleared before rendering the new context.

## Trade Interaction
The **Market Tab** specifically follows a selection-based trade pattern:
- **Up/Down (W/S)**: Change the selected commodity.
- **Left/Right (A/D)**: Adjust the **selectedQuantity** for the transaction (Buy/Sell).
- **Buy (B)**: Execute `EconomyManager::executeTrade` for the current selection and quantity.
- **Sell (V)**: Execute `EconomyManager::executeTrade` with a negative delta for the current selection and quantity.

## Visual Feedback & Observability
- **Total Cost**: A dynamic display in the panel that updates as `selectedQuantity` is adjusted.
- **Cargo Volume**: A persistent indicator (e.g., `45 / 100`) that highlights current ship capacity and provides immediate feedback on trade feasibility.

## Traceability
- **Implementation**: `src/rendering/LandingScreen.cpp`
- **Logic**: `src/game/EconomyManager.cpp`
- **Related Patterns**: [Module Composition](./ship-modular-composition.md)
