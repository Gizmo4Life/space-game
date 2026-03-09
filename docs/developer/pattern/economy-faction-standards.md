---
id: economy-faction-standards
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Economy Faction Standards

# Pattern: Economy Faction Standards

**Intent:** Drive technological progression and dynamic market differentiation by allowing factions to globally capture and retain their best procedurally generated modular layouts, simulating R&D.

## Shape

```cpp
// Within the production resolution logic
auto* fData = FactionManager::instance().getFactionPtr(factionId);
bool isNewStandard = true;

// Compare against current standard
if (fData && fData->factionDesigns.count(product)) {
    int currentScore = calculateScore(fData->factionDesigns[product]);
    if (newScore <= currentScore) {
        isNewStandard = false;
    }
}

// Retain standard, sell excess
if (isNewStandard && fData) {
    if (fData->factionDesigns.count(product)) {
        eco.shopModules.push_back(fData->factionDesigns[product]);
    }
    fData->factionDesigns[product] = generated;
} else {
    // Add inferior/excess production to local market yard
    eco.shopModules.push_back(generated);
}
```

## Key Constraints
- **Global Retention:** The best designs (`ModuleDef` or `AmmoDef`) must be retained globally within the `FactionData` instance so they are accessible to outfitting logic across all star systems.
- **Scrap Yard Market:** Any items that do not beat the current standard (or the old standard once dethroned) are pushed to the `PlanetEconomy`'s `shopModules` or `shopAmmo` collections, serving as the local "Scrap Yard" market for players to purchase.
- **Scoring Definition:** The heuristic for "best" must comprehensively weight product attributes (e.g., Tier ranks, specialized traits).

## Applied In
- `EconomyManager::processProduction` — Evaluates generated items, caches standards, and populates the planetary scrap yard.
- `ShipOutfitter::generateBlueprint` — Consults `FactionData->factionDesigns` to guarantee NPC ships are built with faction-standard equipment rather than random rolls.
