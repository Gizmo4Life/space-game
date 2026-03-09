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
1. **Hull Selection:** Define the base physical constraints using `HullDef` (Mass, Volume, Mounts, Hardpoints, Visual Styles).
2. **Module Allocation:** Attach function-specific modules (Engines, Weapons, Shields, Ammo Racks) to mounts or hardpoints via `ShipOutfitter`.
3. **Attribute-Driven Stat Computation:** Aggregate tiered attributes (`AttributeType` like Size, Mass, Thrust, Capacity) to compute final `ShipStats`. Scaling follows Tier-based multipliers (e.g., T1=1x, T2=3x, T3=8x). Includes capacity calculations for dynamic `InstalledAmmo` inventory storage.
4. **Procedural Rendering:** Generate ship visuals on-the-fly in `RenderSystem` by compositing the hull style with per-slot nacelle/mount styles and connecting them with functional outriggers.

## 3. Data Flow & Integrity
- **Trigger:** Ship market purchase, shipyard outfitting, or NPC spawn event (e.g., `WorldLoader::spawnPlayer`).
- **Output:** A composed ship entity with `HullDef`, tiered module components, and procedurally generated visuals.
- **Consistency:** Atomic transactional outfitting using `defaultOutfits_` per Tier to ensure balanced starting states.

## 4. Operational Context
- **Primary Modules:** [game-core](/docs/architecture/module/game-core.md), [game-economy](/docs/architecture/module/game-economy.md) (T3)
- **Critical Failure Metric:** Ship spawning with negative mass, or energy draw exceeding recharge by 1000% causing immediate shutdown.
