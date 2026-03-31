---
id: ship-generation
type: doc-t3-module
pillar: game
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Ship Generation

# Module: Ship Generation & Evolution

Mapping of the procedural ship outfitting and evolutionary systems.

## 1. Context & Intent
The Ship Generation system is responsible for creating diverse, role-optimized vessel blueprints for NPCs and shipyards. It utilizes an evolutionary loop where ship lines mutate and improve over time based on mission success.

## 2. Physical Mapping
- **Manager**: [ShipOutfitter.cpp](file:///Users/Dan/repos/space-game/src/game/ShipOutfitter.cpp)
- **Fitness Utility**: [ShipFitness.h](file:///Users/Dan/repos/space-game/src/game/components/ShipFitness.h)
- **Hull Logic**: [HullGenerator.cpp](file:///Users/Dan/repos/space-game/src/game/components/HullGenerator.cpp)
- **Module Logic**: [ModuleGenerator.cpp](file:///Users/Dan/repos/space-game/src/game/components/ModuleGenerator.cpp)

## 3. Evolutionary Loop
1. **Candidate Selection**: `ShipOutfitter` generates 4 candidates per request and selects the fittest based on `ShipFitness`.
2. **Fitness Logic**:
    - **Combat**: Requires at least one weapon (else fitness = 0). Base score from weapon attributes (Caliber, ROF, Range).
    - **Trade/Transport**: Driven by `Cargo` and `Habitation` capacities.
    - **Multipliers**: Base scores are multiplied by bonuses for Armor (Hull Tier) and Speed (TWR).
3. **Generational Shifting**: If a ship line's K/D ratio falls below 0.5, `NPCShipManager` triggers `FactionManager::evolveShipLine`.
4. **Edit-Distance Mutation**: `HullGenerator::mutateHull` creates a successor generation by applying small changes (slots, mass, volume) to the previous design.

## 4. Pattern Composition
- [greedy-fitness-generation-retry](/docs/developer/pattern/greedy-fitness-generation-retry.md) (P) — Multi-candidate generation with fitness ranking.
- [deterministic-attribute-tiering](/docs/developer/pattern/deterministic-attribute-tiering.md) (P) — Integer-based tier scaling over random quality rolls.
- [evolutionary-strategy-drift](/docs/developer/pattern/evolutionary-strategy-drift.md) (P) — Stochastic DNA mutation based on performance metrics.
- [blueprint-round-trip](/docs/developer/pattern/blueprint-round-trip.md) (P) — Symmetry between `applyBlueprint` and `blueprintFromEntity`.
- [single-source-calculation](/docs/developer/pattern/single-source-calculation.md) (P) — Stats derived in `refreshStats` only.

## 5. Telemetry & Observability
- `game.generation.blueprint.generate`: Basic generation span. Attributes: `vessel.faction`, `vessel.tier`, `vessel.role`.
- `game.generation.blueprint.apply`: Application span. Attributes: `vessel.entity`.
- `game.generation.stats.refresh`: Live stat recalculation.
- `game.factions.line.evolve`: Triggered on generational shifts.
- `vessel.fitness`: Reported in `game.generation.blueprint.apply`.

## 6. Constraints
- Ships must maintain at least 1 Engine, 1 Command, and 1 Hardpoint slot.
- Power draw must be $\leq 0$ (net generation).
- Internal volume must not be exceeded.
