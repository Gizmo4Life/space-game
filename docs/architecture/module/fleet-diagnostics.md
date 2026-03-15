---
id: fleet-diagnostics-dashboard
type: dashboard
pillar: operational
---

# Dashboard: Fleet Status & Logistics

This manifest defines the observability view for managing the player's ship fleet and mission longevity.

## 1. Trace Spans
| Span Name | Description | Nuance |
| :--- | :--- | :--- |
| `game.ui.fleet_overlay.draw` | HUD rendering cost for fleet status. | Monitor for performance spikes with large fleets. |
| `game.core.ship.stats.refresh` | Calculation of TTE and resource depletion. | Core logic for depletion prediction. |

## 2. Metrics & Probes
- **Fleet Population**: `game.npc.fleet_count`
- **Resource Depletion**: `game.core.ship.tte.minutes`
- **Battery Efficiency**: `game.core.ship.battery.charging_ratio`

## 3. Runbooks
- **TTE Inaccuracy**: If fleet ships exhaust resources faster than predicted, verify `ShipOutfitter::refreshStats` charging logic.
- **Visibility HUD Drift**: If fleet ships are not rendered, audit `NPCShipManager` for missing `Faction` components.
