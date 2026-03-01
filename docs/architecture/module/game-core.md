---
id: game-core-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Core

# Module: Game Core

Procedural world generation, player spawning, and modular vessel outfitting management.

## 1. Physical Scope
- **Path:** `/src/game/` — `WorldLoader.h/.cpp`, `ShipOutfitter.h/.cpp`
- **Components:** `HullDef`, `ShipModule`, `ShipStats`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T2)
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)

## 3. Key Systems
- **ShipOutfitter**: Centralized manager for applying modular outfits to hulls. Maps `VesselClass` and `Faction` to initial configurations. Handles the composition of `HullDef` and `ShipModule` components.
- **ModuleRegistry**: Singleton catalogue for all available ship modules (Engines, Weapons, Shields, etc.). Stores base stats like `thrust`, `damage`, and `volumeCost`.
- **WorldLoader**: Procedural star system generation (stars, planets, moons) and player entity initialization.

## 4. Pattern Composition
- [ship-modular-composition](/docs/developer/pattern/ship-modular-composition.md) (P) — Composition of hulls and modules
- [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) (P) — Star system seeding
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `ShipOutfitter::instance()`
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `HullDef`, `ShipModule`, `ShipStats` PODs

## 5. Telemetry & Observability
- `game.core.world.load` — duration of procedural generation
- `game.core.ship.outfit` — attributes: `vessel.class`, `v_faction_id`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
