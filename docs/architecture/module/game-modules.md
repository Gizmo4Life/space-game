---
id: game-modules
type: doc-t3-module
pillar: game
---

# Module: Ship Modules System

The Ship Modules system is the core functional composition layer of the game, allowing for diverse vessel capabilities through a tiered, attribute-driven architecture.

## 1. Module Categories
Modules are classified into categories that determine their slot placement and functional role:

- **Engine**: Provides `Thrust` and `Efficiency`. Scale with hull size.
- **Weapon**: Offense capabilities (Energy, Projectile, Missile). Key attributes: `Caliber`, `ROF`, `Range`.
- **Shield**: Energy-based defense. Key attributes: `Capacity`, `Regen`.
- **Habitation**: [NEW] Supports population (`crewPopulation` + `passengerPopulation`). Consumes food and power when occupied.
- **Cargo**: [NEW] Dedicated storage for commodities and resources.
- **Reactor/Battery**: Power generation and storage.
- **Command**: Mandatory module for ship operation. Influences `minCrew` requirements.
- **Utility**: Generic support modules.

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

## 4. Economic Fitness
Ship designs are evolved based on their specialization:
- **Combat Fitness**: Aggregated weapon stats multiplied by Armor and Speed bonuses. Requires $\geq 1$ weapon.
- **Trade Fitness**: Driven by Cargo volume capacity.
- **Transport Fitness**: Driven by Habitation passenger capacity.
- **Synergy Multipliers**: High Speed (TWR) and heavy Armor provide compounding bonuses to a ship's role-specific score.

## 5. Physical Scope
- **Definitions**: [ShipModule.h](file:///Users/Dan/repos/space-game/src/game/components/ShipModule.h)
- **Generation**: [ModuleGenerator.cpp](file:///Users/Dan/repos/space-game/src/game/components/ModuleGenerator.cpp)
- **Fitting**: [ShipOutfitter.cpp](file:///Users/Dan/repos/space-game/src/game/ShipOutfitter.cpp)
- **Scoring**: [ShipFitness.h](file:///Users/Dan/repos/space-game/src/game/components/ShipFitness.h)
