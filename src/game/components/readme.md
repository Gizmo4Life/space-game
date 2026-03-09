# /src/game/components/

→ [T3 Module: Game Core](/docs/architecture/module/game-core.md)
→ [T3 Module: Game Economy](/docs/architecture/module/game-economy.md)
→ [T3 Module: Game Factions](/docs/architecture/module/game-factions.md)
→ [T3 Module: Game NPC](/docs/architecture/module/game-npc.md)
→ [T3 Module: Engine Combat](/docs/architecture/module/engine-combat.md)
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)

## Components
- `ShipConfig.h` — Static hull and outfit registry
- `HullDef.h` — Shape, mass, and hardpoint geometry
- `ShipModule.h` — Module attributes and ID definitions
- `Economy.h`- [x] Resolve `EconomyManager` Errors
- [x] Resolve `LandingScreen.cpp` Errors
- [x] Resolve Linking Errors
- [x] Implement Economy Features
    - [x] Dynamic ship pricing based on hull class scarcity
    - [x] Multi-faction planetary seeding (3-5 factions)
    - [x] UI: Display seller faction in Shipyard and Outfitter
    - [x] Market: Aggregate resource and module pools (scrapyards)
    - [x] Shipyard: Functional Sell menu via Tab key
- [x] Final Build Verification
- `WorldConfig.h` — Star system constants
