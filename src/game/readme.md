# Game

→ [T3 Module: Game Economy](/docs/architecture/module/game-economy.md)
→ [T3 Module: Game Factions](/docs/architecture/module/game-factions.md)
→ [T3 Module: Game NPC](/docs/architecture/module/game-npc.md)
→ [T3 Module: Game Combat](/docs/architecture/module/game-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Systems
- `EconomyManager` — Planet production, dynamic pricing, competitive ship market (`getShipBids`, `buyShip`)
- `FactionManager` — Faction data, relationship matrix, credit accumulation
- `NPCShipManager` — NPC spawning, AI state machine, player fleet boids
- `WorldLoader` — Procedural star system and player entity generation
- `TradeManager` — Ship-to-planet resource transactions
- `components/` — ECS component structs; see [components/readme.md](components/readme.md)

## Build
```bash
cmake --build build --target SpaceGame
```
