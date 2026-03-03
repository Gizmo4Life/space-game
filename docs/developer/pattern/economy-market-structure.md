# Pattern: Economy Market Structure

## Context
The ship market previously used a simplified "lowest bid per tier" model. As factions became more differentiated via DNA axes, it became necessary to show all available offers to allow players to choose based on ship role, faction relationship, and price.

## Geometry
The market is populated by `EconomyManager::getHullBids`, which returns a `std::vector<DetailedHullBid>`.

### Data Flow
1. **World Generation**: `WorldLoader::seedEconomy` initializes `PlanetEconomy` on planets and moons.
2. **Offer Aggregation**: `EconomyManager` scans `FactionEconomy::fleetPool` for all factions on a body.
3. **UI Display**: `LandingScreen` renders these bids in a scrollable list, highlighting the player's selection.
4. **Purchase**: `EconomyManager::buyShip` deducts credits and spawns the specific vessel from the faction's pool.

## Visual Design
- **Color Coding**: Offers are colored by faction for brand recognition.
- **Preview Panel**: Shows a static statistical summary of the selected ship before purchase.
- **Controls**: `W/S` or `Up/Down` for navigation; `Enter` or `B` for acquisition.

## Implementation Details
- **Moons**: Now share the same `PlanetEconomy` component as planets but with a scaled-down population (`0.2f`).
- **DetailedHullBid**:
  ```cpp
  struct DetailedHullBid {
    uint32_t factionId;
    Tier tier;
    std::string role;
    float price;
  };
  ```
