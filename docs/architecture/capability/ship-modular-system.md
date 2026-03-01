---
id: ship-modular-system
type: capability
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Capability](/docs/architecture/capability/readme.md) > Ship Modular System

# Capability: Ship Modular System

## 1. Business Intent
Transform the static, role-based vessel model into a flexible, dynamic composition system. This enables players and NPCs to outfit ships for specialized roles (Combat, Trade, Exploration) using atomic hulls and modules, while ensuring performance stats remain deterministic and balanced.

## 2. Orchestration Flow
1. **Hull Selection:** Define the base physical constraints using `HullDef` (Mass, Volume, Mounts, Hardpoints).
2. **Module Allocation:** Attach function-specific modules (Engines, Weapons, Shields) to mounts or hardpoints via `ShipOutfitter`.
3. **Stat Computation:** Aggregate total mass, energy draw, and volume to compute final `ShipStats` each frame.
4. **Validation:** Ensure the final configuration passes physics and energy recharge constraints.

## 3. Data Flow & Integrity
- **Trigger:** Ship market purchase, shipyard outfitting, or NPC spawn event.
- **Output:** A composed ship entity with `HullDef` and multiple `ShipModule` components.
- **Consistency:** Atomic transactional outfitting (check requirements → debit → apply). No "partial outfits" allowed in the registry.

## 4. Operational Context
- **Primary Modules:** [game-core](/docs/architecture/module/game-core.md), [game-economy](/docs/architecture/module/game-economy.md) (T3)
- **Critical Failure Metric:** Ship spawning with negative mass, or energy draw exceeding recharge by 1000% causing immediate shutdown.
