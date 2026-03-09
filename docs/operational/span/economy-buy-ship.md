---
id: span-economy-buy-ship
type: span_runbook
module: game-economy-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > economy.buy_ship

# Span Runbook: economy.buy_ship

## 1. Component Scope
- **Module:** [Game Economy](/docs/architecture/module/game-economy.md)
- **Source:** `EconomyManager::buyShip`
- **Description:** Tracks ship ownership transfer, flagship swapping, and credit deduction.

## 2. Diagnostic Mapping
| Metric | Threshold | logic |
| :--- | :--- | :--- |
| **Error Rate** | > 0% | Transaction failure. Potentially missing `NPCComponent` or insufficient credits. |
| **Latency** | > 100ms | Performance bottleneck in flagship entity spawning or fleet re-assignment. |

**Isolate Failure:**
- Check logs for "Set does not contain entity" assertions (EnTT registry drift).
- Verify `player.credits` balance before and after the span.

## 3. Mitigation & Restoration
1. **Rollback Credits:** If the ship was not spawned but credits were deducted, manually increment `CreditsComponent`.
2. **Re-spawn Flagship:** If flagship swap failed, use developer console to force-spawn the vessel for the `player.id`.
3. **Verify Fleet State:** Ensure the old flagship has correctly joined the fleet as an escort (has `NPCComponent` and `leaderEntity` set).

## 4. Recovery Verification
- Success event `game.ui.ship.purchase` recorded with matching `vessel.class`.
