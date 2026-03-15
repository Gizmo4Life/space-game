---
id: span-engine-resource-control-loss
type: span_runbook
module: game-core-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > engine.resource.control_loss

# Span Runbook: engine.resource.control_loss

## 1. Component Scope
- **Module:** [Game Core](/docs/architecture/module/game-core.md)
- **Source:** `ResourceSystem::update`
- **Description:** Tracks vessels losing operational control due to insufficient crew.

## 2. Diagnostic Mapping
| Metric | Threshold | Logic |
| :--- | :--- | :--- |
| **Control Loss Event** | [x] | Crew population fell below `minCrew`. |
| **Derelict Status** | [x] | All crew died; ship is now an inert hulk. |

**Isolate Failure:**
- Check `reason` attribute (e.g., `insufficient_crew`).
- Correlate with `engine.resource.death` spans to see if depletion caused the loss.

## 3. Mitigation & Restoration
1. **Add Crew**: Transfer crew members to the derelict or under-crewed vessel.
2. **Reclaim Vessel**: If derelict, a boarding party can restore basic systems and take over the ship.
3. **Tug to Port**: Use a tractor beam or tow ship to bring the inert vessel to a shipyard for refitting.

## 4. Recovery Verification
- Monitor `engine.resource.control_loss` incidents. Control should return once population >= `minCrew`.
