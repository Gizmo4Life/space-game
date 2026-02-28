# /src/game/components/

→ [T3 Module: Game Economy](/docs/architecture/module/game-economy.md)
→ [T3 Module: Game Factions](/docs/architecture/module/game-factions.md)
→ [T3 Module: Game NPC](/docs/architecture/module/game-npc.md)
→ [T3 Module: Game Combat](/docs/architecture/module/game-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Components
- `Economy.h` — `PlanetEconomy`, `CreditsComponent`, `CargoComponent`, `Resource` enum
- `Faction.h` — `Faction` allegiance map per planet/ship entity
- `NPCComponent.h` — AI belief/state machine, fleet flags, decision timers
- `CelestialBody.h` — `CelestialType` enum, orbital parameters
- `WeaponComponent.h` — Cooldown, damage, projectile spec
- `ShipStats.h` — Hull and energy stats
- `NameComponent.h` — Display name for any entity
- `CargoComponent.h` — Cargo hold with resource quantities
- `WorldConfig.h` — `WORLD_SCALE` and `SHIP_SCALE` constants
