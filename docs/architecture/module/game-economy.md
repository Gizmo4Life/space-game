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
- **EconomyManager**: Orchestrates planetary markets and the ship inventory.
  - **Dynamic Pricing**: Uses factory supply nodes to adjust `basePrice` across factions.
  - **Ship Purchase (`buyShip`)**: Handles vessel ownership transfer. If replacing a flagship, it spawns a player-controlled entity, removes its `NPCComponent` safely (verifying pre-existence to avoid crashes), and re-assigns the old flagship as a fleet escort.
- **TradeManager**: Lightweight singleton for executing fine-grained cargo transactions within the UI.

## 4. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `PlanetEconomy`, `CargoComponent`, `CreditsComponent`
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `EconomyManager::update`
- [cpp-compiler-driven-refactoring](/docs/developer/pattern/cpp-compiler-driven-refactoring.md) (P) — Mandatory protocol for wide-reaching structural changes.
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `EconomyManager`, `TradeManager`
- [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) (P) — Resource seeding via `WorldLoader`
- [economy-resource-chain](/docs/developer/pattern/economy-resource-chain.md) (P) — Three-tier production chain (Basic -> Refined -> Modular)
- [dna-weighted-infrastructure-expansion](/docs/developer/pattern/dna-weighted-infrastructure-expansion.md) (P) — Faction factory construction based on "Need" and DNA axis bias
- [economy-dynamic-pricing](/docs/developer/pattern/economy-dynamic-pricing.md) (P) — Dynamic assignment of `basePrice` utilizing material costs
- [economy-faction-standards](/docs/developer/pattern/economy-faction-standards.md) (P) — Modular designs compete to become faction standards. Best variants go to `FactionData`, inferior to local scrap yard.
- [economy-arbitrage-logic](/docs/developer/pattern/economy-arbitrage-logic.md) (P) — AI traders move goods between planets based on price differentials and faction DNA
- [economy-competitive-market](/docs/developer/pattern/economy-competitive-market.md) (P) — Faction bid model using `Tier` for tiered pricing
- [economy-refit-fee](/docs/developer/pattern/economy-refit-fee.md) (P) — Installation costs for player refits
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P)
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)

## 4. Telemetry & Observability
- `game.economy.tick` — attributes: `economy.planet_count`
- `game.economy.trade.transaction` — attributes: `economy.credits_transferred`
- `game.economy.factory.build` — attributes: `economy.product_type`, `economy.product_id`, `economy.cost_credits`
- `game.economy.stockpile.delta` — attributes: `faction.id`, `product.id`, `stockpile.delta`
- **Status:** ✅ Fully instrumented via `opentelemetry-cpp`
