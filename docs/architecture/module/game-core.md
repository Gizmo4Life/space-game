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
- **Components:** `HullDef`, `ShipModule`, `ShipStats`, `ShipConfig`, `AmmoDef`, `InstalledAmmo`, `InstalledHabitation`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T2)
- [Capability: Navigation](/docs/architecture/capability/navigation.md) (T2)

## 3. Key Systems
- **PowerSystem**: Manages energy production via Isotope Reactors (exponential fuel decay) and buffer storage in Batteries. Updates `batteryLevel` for energy weapon draw.
- **ResourceSystem**: Implements persistence and survival logic. Tracks Food, Fuel, Isotopes, and Ammo consumption. Enforces consequences for depletion: Starvation (population loss), Power Failure (Life Support death timer), and Control Loss (minimum crew requirements).
- **ShipOutfitter**: Centralized manager for applying modular outfits to hulls. Uses a **Unified Slot System** where modules are mapped to `SlotRole` types. Handles technical validation: ensure at least one `Command` slot is present, manages power balance, and enforces volume bounds for `AmmoMagazine`. Enforces **Vessel Viability Standard**: All generated ships must have at least 5 days of Time to Exhaustion (TTE) for critical resources, utilizing role and aggression-based **mission profiling** for ammunition capacity sizing. Calculates `rotationSpeed` using a balanced `baseTurnRate` (1500) for softer steering feel. **Persistent Economy Support**: Includes `applyBlueprint` overloads to support spawning ships from pre-calculated blueprints and automatically populates initial resources based on the 5-day TTE target.
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
- `engine.resource.starvation` — attributes: `vessel.entity`, `vessel.deaths`
- `engine.resource.isotope_depletion` — attributes: `vessel.entity`
- `engine.resource.power_failure_death` — attributes: `vessel.entity`, `vessel.deaths`
- `engine.resource.derelict` — attributes: `vessel.entity`
- `engine.resource.control_loss` — attributes: `vessel.entity`, `reason`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0
