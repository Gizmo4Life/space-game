# Game

→ [Home](/docs/readme.md)
→ [T3 Module: Game Economy](/docs/architecture/module/game-economy.md)
→ [T3 Module: Game Core](/docs/architecture/module/game-core.md)
→ [T3 Module: Game Factions](/docs/architecture/module/game-factions.md)
→ [T3 Module: Game NPC](/docs/architecture/module/game-npc.md)
→ [T3 Module: Engine Combat](/docs/architecture/module/engine-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Logic Encapsulation](/docs/governance/standard/logic-encapsulation-standard.md)
→ [Standard: State Synchronization](/docs/governance/standard/state-synchronization.md)
→ [Standard: Logic & Test Integrity](/docs/governance/standard/logic-test-integrity.md)

  ## Systems
- `EconomyManager` — Planet production, dynamic pricing, and persistent physical ship/module inventories.
- `FactionManager` — Faction DNA drift, relationship matrix, and per-outpost credit accumulation.
- `NPCShipManager` — NPC spawning from blueprints, AI state machine, and mission success tracking.
- `HullGenerator` — [T3 Module: Game Hull](/docs/architecture/module/game-hull.md) — Procedural builder for role-based vessel hulls.
- `ShipOutfitter` — Modular vessel composition; handles persistent stock application, starter ammo initialization, and centralized blueprint extraction (`blueprintFromEntity`).
- `WorldLoader` — Procedural star system generation and initial fleet seeding.
- `TradeManager` — Ship-to-planet resource transactions.
- `components/` — ECS component structs; see [components/readme.md](components/readme.md)

## Coding Standards
- **Blueprint Extraction**: Use `ShipOutfitter::blueprintFromEntity` — never manually aggregate `InstalledModules` components. See [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md).
- **Player Lookup**: Use `findFlagship(registry)` — never inline `registry.view<PlayerComponent>()`. See [ghost-logic](/docs/developer/pattern/ghost-logic.md).
- **Blueprint Symmetry**: Any system that applies and extracts blueprints must have a round-trip test. See [blueprint-round-trip](/docs/developer/pattern/blueprint-round-trip.md).
- **Header Hygiene**: Remove orphaned includes immediately after centralizing logic. See [header-management](/docs/governance/standard/header-management.md) § Post-Refactor Orphan Hygiene.

## Build
```bash
cmake --build build --target SpaceGame
```
