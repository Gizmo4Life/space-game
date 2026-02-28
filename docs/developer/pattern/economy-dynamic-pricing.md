---
id: economy-dynamic-pricing
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Economy Dynamic Pricing

# Pattern: Economy Dynamic Pricing

**Intent:** Derive market prices from real-time supply scarcity so that shortages raise prices and surpluses lower them automatically.

## Shape

```cpp
// Recalculate price each tick per resource per planet
float recalcPrice(float targetStock, float currentStock, float basePrice) {
    if (currentStock <= 0.f) return basePrice * MAX_MULTIPLIER;
    return basePrice * (targetStock / currentStock);
}
```

## Key Constraints
- **Formula:** `price = basePrice × (targetStock / currentStock)`.
- **Oscillation guard:** Price change per tick must not exceed 50 % of the previous value.
- **Zero-stock guard:** When `currentStock == 0`, price clamps to `basePrice × MAX_MULTIPLIER` rather than dividing by zero.
- **Deterministic:** Recalculated every tick from live stockpile data; no historical smoothing.

## Applied In
- `EconomyManager::update` — `PlanetEconomy::currentPrices` recalculated each frame.
- `LandingScreen::drawPlanetInfo` — Prices displayed to the player at landing.
