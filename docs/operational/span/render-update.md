---
id: span-render-update
type: span_runbook
module: rendering-main-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > render.update

# Span Runbook: render.update

## 1. Component Scope
- **Module:** [Rendering Main](/docs/architecture/module/rendering-main.md)
- **Source:** `RenderSystem::update`
- **Description:** Tracks the duration of the 4-pass SFML rendering pipeline.

## 2. Diagnostic Mapping
| Metric | Threshold | logic |
| :--- | :--- | :--- |
| **Latency** | > 16.6ms | Frame drop (below 60 FPS). Likely over-draw or too many entities. |
| **Error Rate** | > 1% | SFML texture/buffer allocation failures. |

**Isolate Failure:**
- Check `engine.rendering.indicator.count` to see if offscreen indicator logic is ballooning.
- Filter by `planet.id` to see if specific high-density star systems are causing the spike.

## 3. Mitigation & Restoration
1. **Reduce Entity Density:** If latency is due to entity count, trigger culling logic or reduce background sprite counts.
2. **Clear Texture Cache:** If error rate is high, restart the `MainRenderer` to flush SFML texture memory.
3. **Toggle Offscreen Indicators:** Disable T3 UI layer indicators if they are the primary bottleneck.

## 4. Recovery Verification
- Monitor `engine.rendering.update` duration. Healthy state is < 8ms.
