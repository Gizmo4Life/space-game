---
id: economy-competitive-market
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Docs](/docs/developer/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Economy Competitive Market

# Pattern: Economy Competitive Market

**Intent:** Allow multiple factions to compete for a buyer's business by submitting supply-adjusted bids; the cheapest valid bid wins, and the purchase is an atomic credit transfer.

## Shape

```cpp
// 1. Collect bids: one per faction with supply at this planet
map<uint32_t, float> getShipBids(registry, planet, vesselType) {
    for (faction with fleetPool[type] > 0 at planet) {
        float supplyFactor = lerp(0.70f, 1.30f, stockLevel);
        bids[factionId] = BASE_PRICE * supplyFactor;
    }
    return bids;
}

// 2. Atomic purchase: cheapest bidder wins
void buyShip(registry, planet, player, type, worldId) {
    auto [winnerFactionId, price] = min_element(bids);
    assert(player.credits >= price);        // affordability gate
    player.credits  -= price;               // debit buyer
    winner.credits  += price;               // credit seller
    winner.fleetPool[type]--;               // decrement supply
    spawnFleetNPC(player, winnerFactionId); // spawn ship
}
```

## Key Constraints
- **Supply factor range:** `[0.70, 1.30]` — higher stock lowers price, scarcity raises it.
- **Atomicity:** All four steps (gate → debit → credit → spawn) complete or none do; no partial state.
- **Cheapest wins:** Player always buys from the lowest bidder; no faction preference.
- **Fleet integration:** Spawned ship is tagged `isPlayerFleet = true`, `leaderEntity = player`.

## Applied In
- `EconomyManager::getShipBids` — Bid collection per vessel type.
- `EconomyManager::buyShip` — Atomic purchase and NPC spawn.
- `LandingScreen::drawShipMarket` — UI rendering of sorted bids.
