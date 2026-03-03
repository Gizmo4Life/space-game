---
id: game-core-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Core

# Module: Game Core

Procedural world generation, player spawning, and modular vessel outfitting management.

## 1. Physical Scope
- **Path:** `/src/game/` — `WorldLoader.h/.cpp`, `ShipOutfitter.h/.cpp`, `components/ShipConfig.h`
- **Components:** `HullDef`, `ShipModule`, `ShipStats`, `ShipConfig`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T2)
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)

## 3. Key Systems
- **ShipOutfitter**: Centralized manager for applying modular outfits to hulls. Uses `Tier` to determine base attributes and utility slot counts. Handles the composition of `HullDef` and `ShipModule` components. Enforces technical validation: vessels must include at least one reactor, maintain a positive power balance, and stay within internal volume bounds.
- **ShipConfig**: Static registry of hull definitions and default outfits. Replaces hardcoded mappings within the outfitter to allow for data-driven ship balancing.
- **ModuleRegistry**: Singleton catalogue for all available ship modules.
- **WorldLoader**: Procedural star system generation and deterministic player spawning near inhabited worlds.

## 4. Pattern Composition
- [ship-modular-composition](/docs/developer/pattern/ship-modular-composition.md) (P) — Composition of hulls and modules
- [tiered-utility-allocation](/docs/developer/pattern/tiered-utility-allocation.md) (P) — Scaling slot counts by vessel tier
- [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) (P) — Star system seeding with orbital pre-calculation
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `ShipOutfitter::instance()`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `HullDef`, `ShipModule`, `ShipStats` PODs

## 5. Telemetry & Observability
- `game.core.world.load` — duration of procedural generation
- `game.core.ship.outfit` — attributes: `vessel.class`, `v_faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
