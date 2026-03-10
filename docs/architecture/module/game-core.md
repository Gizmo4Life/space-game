---
id: game-core-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Core

# Module: Game Core

Procedural world generation, player spawning, and modular vessel outfitting management.

## 1. Physical Scope
- **Path:** `/src/game/` — `WorldLoader.h/.cpp`, `ShipOutfitter.h/.cpp`, `components/ShipConfig.h`, `components/ModuleGenerator.h/.cpp`
- **Components:** `HullDef`, `ShipModule`, `ShipStats`, `ShipConfig`, `AmmoDef`, `InstalledAmmo`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T2)
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)

## 3. Key Systems
- **ShipOutfitter**: Centralized manager for applying modular outfits to hulls. Uses a **Unified Slot System** where modules are mapped to `SlotRole` types. Handles technical validation: ensure at least one `Command` slot is present, manages power balance, and enforces volume bounds for `AmmoMagazine`. Implement relaxed **Module Tier Logic** (70% match for non-elites) and **Attribute Variance** (15% chance to roll +1 tier) to ensure competitive common vessels. Also calculates `rotationSpeed` using a balanced `baseTurnRate` (1500) for softer steering feel.
- **ModuleGenerator**: Procedural factory for creating `ModuleDef` and `AmmoDef` variants. Scales attributes (Mass, Volume, Thrust, Output) based on Tier and Faction DNA.
- **PowerSystem**: Manages energy production via Isotope Reactors (exponential fuel decay) and buffer storage in Batteries. Updates `batteryLevel` for energy weapon draw.
- **ShipConfig**: Static registry of hull definitions and default outfits. Replaces hardcoded mappings within the outfitter to allow for data-driven ship balancing.
- **ModuleRegistry**: Singleton catalogue for all available ship modules.
- **WorldLoader**: Procedural star system generation and deterministic player spawning near inhabited worlds.

## 4. Pattern Composition
- [ship-modular-composition](/docs/developer/pattern/ship-modular-composition.md) (P) — Composition of hulls and modules
- [tiered-utility-allocation](/docs/developer/pattern/tiered-utility-allocation.md) (P) — Scaling slot counts by vessel tier
- [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) (P) — Star system seeding with orbital pre-calculation
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `ShipOutfitter::instance()`
- [cpp-interface-segregation](/docs/developer/pattern/cpp-interface-segregation.md) (P) — Used by outfitting UI panels to support off-screen rendering targets.
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `HullDef`, `ShipModule`, `ShipStats` PODs

## 5. Telemetry & Observability
- `game.core.world.load` — duration of procedural generation
- `game.core.ship.outfit` — attributes: `vessel.class`, `v_faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
