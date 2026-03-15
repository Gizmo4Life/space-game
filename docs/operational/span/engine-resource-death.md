---
id: span-engine-resource-death
位type: span_runbook
module: game-core-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > engine.resource.death

# Span Runbook: engine.resource.death

## 1. Component Scope
- **Module:** [Game Core](/docs/architecture/module/game-core.md)
- **Source:** `ResourceSystem::update`
- **Description:** Tracks crew deaths due to starvation or power failure (life support).

## 2. Diagnostic Mapping
| Metric | Threshold | Logic |
| :--- | :--- | :--- |
| **Total Deaths** | > 0 | Ships are running out of Food or Isotopes. |
| **Isotope Depletion** | [x] | Power loss leading to life support failure. |

**Isolate Failure:**
- Check `vessel.entity` attribute to identify the specific ship.
- Check `vessel.deaths` to see the scale of the population loss.
- Review `engine.resource.isotope_depletion` spans to see if power failure preceded death.

## 3. Mitigation & Restoration
1. **Resupply Vessel**: Transfer Food or Isotopes to the affected ship via trader exchange or fleet resupply.
2. **Evacuate Crew**: If the ship is beyond resupply, evacuate survivng passengers/crew to a healthy vessel.
3. **Check Consumption Rates**: Verify if `GAME_SECONDS_PER_DAY` has been altered, causing unnatural depletion.

## 4. Recovery Verification
- Monitor `engine.resource.starvation` and `engine.resource.power_failure_death`. Healthy state is 0 deaths.
