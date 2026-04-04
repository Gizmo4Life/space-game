---
id: operational-span-manifest
type: manifest
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > Span

# Sub-pillar: Span

Operational runbooks for specific telemetry spans, providing diagnostics and restoration steps.

## Rendering Spans
- [render.update](render-update.md): Latency and error tracking for the SFML rendering pipeline.

## Economy Spans
- [economy.buy_ship](economy-buy-ship.md): Transaction and flagging logic for ship purchases.
- [economy.transaction](economy-transaction.md): Commodity buy/sell operations and fleet-scale trade logistics.
- [economy.stockpile.imbalance](economy-stockpile-imbalance.md): Planetary supply chain anomalies and resource stockpile drift.

## Combat Spans
- [combat.weapon.fire](combat-weapon-fire.md): Projectile spawning and ammo/power consumption tracking.

## NPC Spans
- [npc.mission.stagnation](npc-mission-stagnation.md): NPC mission throughput degradation and behavioral stalling.

## World Generation Spans
- [world.gen.anomaly](world-gen-anomaly.md): Procedural generation performance and density anomalies.

## Engine Spans
- [engine.resource.death](engine-resource-death.md): Crew death and vessel loss from resource starvation.
- [engine.resource.control_loss](engine-resource-control-loss.md): Loss of vessel control from fuel or power depletion.
