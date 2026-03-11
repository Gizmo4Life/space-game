# Game

→ [Home](/docs/readme.md)
→ [T3 Module: Game Economy](/docs/architecture/module/game-economy.md)
→ [T3 Module: Game Core](/docs/architecture/module/game-core.md)
→ [T3 Module: Game Factions](/docs/architecture/module/game-factions.md)
→ [T3 Module: Game NPC](/docs/architecture/module/game-npc.md)
→ [T3 Module: Engine Combat](/docs/architecture/module/engine-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

  ## Systems
- `EconomyManager` — Planet production, dynamic pricing, and persistent physical ship/module inventories.
- `FactionManager` — Faction DNA drift, relationship matrix, and per-outpost credit accumulation.
- `NPCShipManager` — NPC spawning from blueprints, AI state machine, and mission success tracking.
- `HullGenerator` — [T3 Module: Game Hull](/docs/architecture/module/game-hull.md) — Procedural builder for role-based vessel hulls.
- `ShipOutfitter` — Modular vessel composition; handles persistent stock application and starter ammo initialization.
- `WorldLoader` — Procedural star system generation and initial fleet seeding.
- `TradeManager` — Ship-to-planet resource transactions.
- `components/` — ECS component structs; see [components/readme.md](components/readme.md)

## Build
```bash
cmake --build build --target SpaceGame
```
