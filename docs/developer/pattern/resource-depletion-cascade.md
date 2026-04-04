---
id: resource-depletion-cascade
type: pattern
pillar: developer
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Resource Depletion Cascade

# Pattern: Resource Depletion Cascade

A system consumes a resource each tick. When the resource reaches zero, a tiered chain of consequences fires, where each tier activates only after its predecessor's resource is exhausted.

## Structure

1. **Consumption Tick**: A rate-based draw reduces the resource stock each frame.
2. **Depletion Guard**: When stock reaches zero, consumption stops and consequences begin.
3. **Consequence Tier**: A specific failure mode activates (e.g., population death, system shutdown).
4. **Cascade**: Each consequence may deplete a secondary resource, triggering the next tier.
5. **Telemetry Probe**: Each consequence tier emits a distinct span for observability.
