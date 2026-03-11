---
id: span-npc-mission-stagnation
type: span_runbook
module: game-npc-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > npc.mission.stagnation

# Span Runbook: Mission Stagnation

## 1. Component Scope
- **Module:** [NPC Manager](/docs/architecture/module/game-npc.md)
- **Signal:** High active mission count with low `game.npc.mission.outcome` throughput.
- **Description:** Tracks cases where NPCs are deployed but never arrive at destinations or die, causing mission "leakage".

## 2. Diagnostic Mapping
| Metric | Threshold | logic |
| :--- | :--- | :--- |
| **Active Count** | > 50 missions | Excessive memory/compute draw for NPCs. |
| **Outcome Delay** | > 10 min | NPCs stuck in `Traveling` state without reaching destination. |

**Isolate Failure:**
- Check `game.npc.mission.tick` attributes for `mission.active_count`.
- Filter `game.npc.spawn` by `npc.size_tier` to see if large fleets are bottlenecking physics.
- Verify `npc.targetEntity` is still valid in the registry.

## 3. Mitigation & Restoration
1. **Force Terminate:** Use developer console to purge missions with IDs older than 20 minutes.
2. **Reset AI Targets:** If NPCs are circling a planet, force-set their `AIState` back to `Traveling`.
3. **Throttle Spawns:** Increase `SPAWN_INTERVAL` in `NPCShipManager` to 30.0s temporarily.

## 4. Recovery Verification
- `game.npc.mission.tick` shows `active_count` returning to < 20.
- `game.npc.mission.outcome` events resume firing.
