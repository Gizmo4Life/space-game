---
id: span-combat-weapon-fire
type: span_runbook
module: engine-combat-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > combat.weapon.fire

# Span Runbook: combat.weapon.fire

## 1. Component Scope
- **Module:** [Engine Combat](/docs/architecture/module/engine-combat.md)
- **Source:** `WeaponSystem::update`
- **Description:** Tracks projectile instantiation, ammo consumption, and battery draw.

## 2. Diagnostic Mapping
| Metric | Threshold | logic |
| :--- | :--- | :--- |
| **Error Rate** | > 0.1% | Projectile spawn failures or ammo registry mismatches. |
| **Latency** | > 5ms | Rapid-fire weapons causing engine-combat update spikes. |

**Isolate Failure:**
- Verify `InstalledAmmo` count decreases correctly.
- Check `batteryLevel` for energy weapons; if stuck at 0, check [Game Core](/docs/architecture/module/game-core.md) power production.

## 3. Mitigation & Restoration
1. **Fix Ammo Registry:** If ammo consumption is failing, verify `AmmoDef` mapping in `ShipOutfitter`.
2. **Buffer Battery:** If energy weapons are failing due to power, force-charge `batteryLevel` to allow combat continuation.
3. **Throttle ROF:** If latency is spiked, increase weapon cooldowns temporarily.

## 4. Recovery Verification
- `game.combat.weapon.fire` event attributes show successful `projectile_speed` calculation.
