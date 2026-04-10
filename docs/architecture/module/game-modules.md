---
id: game-modules
type: doc-t3-module
pillar: game
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Ship Modules

# Module: Ship Modules System

The Ship Modules system is the core functional composition layer of the game, allowing for diverse vessel capabilities through a tiered, attribute-driven architecture.

## 1. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T2)

## 2. Module Categories
Modules are classified into categories that determine their slot placement and functional role:

- **Engine**: Provides `Thrust` and `Efficiency`. Scale with hull size.
- **Weapon**: Offense capabilities (Energy, Projectile, Missile). Key attributes: `Caliber`, `ROF`, `Range`.
- **Shield**: Energy-based defense. Key attributes: `Capacity`, `Regen`.
- **Habitation**: [NEW] Supports population (`crewPopulation` + `passengerPopulation`). Consumes food and power when occupied.
- **Cargo**: [NEW] Dedicated storage for commodities and resources.
- **Reactor/Battery**: Power generation and storage.
- **Command**: Mandatory module for ship operation. Influences `minCrew` requirements.
- **Utility**: Generic support modules.
- **Ammunition**: Physical storage and management of projectiles and missiles. See [Pattern: Ammunition Management](/docs/developer/pattern/ammunition-management.md).

## 2. Attribute System
Modules use `AttributeType` and `Tier` (T1-T3) to define performance:
- **Volume**: Footprint efficiency (how much internal space it takes).
- **Efficiency**: Multiplier for energy/food consumption and maintenance.
- **Capacity**: Scalable limit for passengers, energy, or shield strength.

## 3. Habitation & Life Support
Vessels must support their population via Habitation modules:
- **Distribution**: The system automatically fills the most efficient habitation modules first.
- **Resource Consumption**:
    - **Power**: Only occupied habitation modules draw power.
    - **Food**: Consumption rate is per-person, influenced by module `Efficiency`.
- **Minimum Crew**: A vessel's `minCrew` requirement is derived from its `Command` modules and overall hull size.
- **Viability Standard**: All generated or refitted ships must maintain a **5-day TTE (Time to Exhaustion)** for Food, Isotopes, and Ammo at default or profiled operational combat levels.

## 4. Economic Fitness & Role Enforcement
Ship designs are evolved and validated based on their specialization. Functional enforcement is applied during outfitting to prevent role-critical module loss:
- **Combat Fitness**: Aggregated weapon stats multiplied by Armor and Speed bonuses.
    - *Requirement:* Must have $\geq 1$ weapon module.
    - *Protection:* Weapons cannot be removed during auto-balancing for power or volume.
- **Trade Fitness**: Driven by Cargo volume capacity.
    - *Requirement:* Must have $\geq 1$ cargo module.
    - *Protection:* Cargo pods are preserved during volume balancing.
- **Transport Fitness**: Driven by Habitation passenger capacity.
    - *Requirement:* Must have $\geq 1$ habitation module.
    - *Protection:* Habitation modules are preserved during volume balancing.
- **Synergy Multipliers**: High Speed (TWR) and heavy Armor provide compounding bonuses to a ship's role-specific score.

## 5. Pattern Composition
- [ship-modular-composition](/docs/developer/pattern/ship-modular-composition.md) (P) — Modular assembly of hulls and modules
- [tiered-utility-allocation](/docs/developer/pattern/tiered-utility-allocation.md) (P) — Scaling capacity and volume by Tier
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `ShipModule` and `InstalledModules` PODs

## 6. Physical Scope
- **Definitions**: [ShipModule.h](file:///Users/Dan/repos/space-game/src/game/components/ShipModule.h)
- **Generation**: [ModuleGenerator.cpp](file:///Users/Dan/repos/space-game/src/game/components/ModuleGenerator.cpp)
- **Fitting**: [ShipOutfitter.cpp](file:///Users/Dan/repos/space-game/src/game/ShipOutfitter.cpp)
- **Scoring**: [ShipFitness.h](file:///Users/Dan/repos/space-game/src/game/components/ShipFitness.h)
