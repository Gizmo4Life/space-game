# Game

→ [T3 Module: Game Economy](/docs/architecture/module/game-economy.md)
→ [T3 Module: Game Combat](/docs/architecture/module/game-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Systems
- `EconomyManager` — Planet production, pricing, competitive ship market (`getShipBids`, `buyShip`)
- `FactionManager` — Faction data, relationships, credit accumulation
- `NPCShipManager` — NPC spawning, AI ticking, fleet boids behavior
- `WorldLoader` — Procedural star system and player entity generation
- `TradeManager` — Ship-to-planet resource transactions
- `components/` — All ECS component struct definitions

## Build
```bash
cmake --build build --target SpaceGame
```
