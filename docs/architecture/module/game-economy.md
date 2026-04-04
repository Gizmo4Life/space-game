---
id: game-economy-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Economy

# Module: Game Economy

Planetary production/consumption simulation, dynamic pricing, trade transactions, and the competitive ship market.

## 1. Physical Scope
- **Path:** `/src/game/` — `EconomyManager.h/.cpp`, `TradeManager.h/.cpp`, `WorldLoader.h/.cpp`
- **Components:** `components/Economy.h`, `components/CargoComponent.h`, `components/InertialBody.h`, `components/ShipStats.h`, `components/NPCComponent.h`, `AmmoDef`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T1) — Refit Fees

## 3. Key Systems
- **EconomyManager**: Orchestrates planetary markets and the persistent ship inventory.
  - **Dynamic Pricing**: Uses factory supply nodes and `hullClassScarcity` to adjust prices. Prices are clamped between 0.1x and 10x of the base price.
  - **Ship Transactions**: `buyShip` and `sellShip` handle vessel ownership. Buying a ship consumes it from the persistent `parkedShips` inventory. Selling a ship adds its hull back to the `scrapyardHulls`.
  - **Ship Exchange**: `transferShipToFaction` allows the player to move an active fleet vessel back into the faction's `parkedShips` pool while landed. This process uses `ShipOutfitter::blueprintFromEntity` to capture the current module configuration as a blueprint — a single centralized extraction point.
  - **Competitive Bidding**: Factions list hulls via `DetailedHullBid`. These bids are now generated directly from the `parkedShips` vector, ensuring the shipyard displays actual physical inventory.
  - **Scrapyard Management**: Factions store salvaged and obsolete inventory in `scrapyardModules` and `scrapyardHulls`.
  - **Ship Assembly**: `tryAssembleShips` greedily combines `scrapyardHulls` and `shopModules` into `ShipBlueprint` objects, which are then added to `parkedShips` for sale. This system ensures that faction production of parts eventually results in available ships.
  - **Resource Trading**: `executeTrade` manages commodity buy/sell between player cargo and aggregate planetary stockpiles.
- **TradeManager**: Lightweight singleton for executing fine-grained cargo transactions within the UI.

## 4. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `PlanetEconomy`, `CargoComponent`, `CreditsComponent`
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)
- [logic-encapsulation-standard](/docs/governance/standard/logic-encapsulation-standard.md) (S) — Context: Fleet-Wide Resource Management
- [fleet-wide-resource-aggregation](/docs/developer/pattern/fleet-wide-resource-aggregation.md) (P) — Preferred for market/reequip
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `EconomyManager::update`
- [cpp-compiler-driven-refactoring](/docs/developer/pattern/cpp-compiler-driven-refactoring.md) (P) — Mandatory protocol for wide-reaching structural changes.
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `EconomyManager`, `TradeManager`
- [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) (P) — Resource seeding via `WorldLoader`
- [economy-resource-chain](/docs/developer/pattern/economy-resource-chain.md) (P) — Three-tier production chain (Basic -> Refined -> Modular)
- [dna-weighted-infrastructure-expansion](/docs/developer/pattern/dna-weighted-infrastructure-expansion.md) (P) — Faction factory construction based on "Need" and DNA axis bias
- [economy-dynamic-pricing](/docs/developer/pattern/economy-dynamic-pricing.md) (P) — Dynamic assignment of `basePrice` utilizing material costs
- [economy-faction-standards](/docs/developer/pattern/economy-faction-standards.md) (P) — Modular designs compete to become faction standards. Best variants go to `FactionData`, inferior to local scrap yard. Includes **Exceptional** module retention (2+ T3 attributes kept for fleets).
- [economy-arbitrage-logic](/docs/developer/pattern/economy-arbitrage-logic.md) (P) — AI traders move goods between planets based on price differentials and faction DNA
- [economy-competitive-market](/docs/developer/pattern/economy-competitive-market.md) (P) — Faction bid model using `Tier` for tiered pricing
- [economy-refit-fee](/docs/developer/pattern/economy-refit-fee.md) (P) — Installation costs for player refits
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)
- [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md) (P) — `ShipOutfitter::blueprintFromEntity` centralizes all entity-to-blueprint extraction; `findFlagship` unifies player identification in refit/sell operations.

### Market Logistics
- **Fleet-Wide Aggregation**: Commodity trading (`executeTrade`) and vessel provisioning (`reequipForDuration`) aggregate cargo capacity and resource stock across the entire active fleet (all ships marked `isPlayerFleet`).
- **Automatic Distribution**: Purchased resources are distributed to ships with available capacity, starting with the flagship.
- **Atomic Transactions**: Trades fail if the aggregate fleet capacity or credits are insufficient.

## 5. Telemetry & Observability
- `game.economy.transaction` — attributes: `transaction_type` (`commodity_buy` / `commodity_sell`), `resource_id`, `quantity`, `price_total`, `fleet_size`, `buying_faction`
- `game.economy.reequip` — attributes: `reequip_days`, `reequip_fleet_size`, `reequip_total_spent`
- **Status:** ✅ Fully instrumented with fleet-scale attributes.

## 6. Economic Balance & Tuning
- **Isotope Availability**: To avoid bottlenecks in power and fuel production, Isotopes are balanced with a higher `baseOutputRate` (15.0f) and increased baseline seeding (4 factories) on `Icy` planets.
