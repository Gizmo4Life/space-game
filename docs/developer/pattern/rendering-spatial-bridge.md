---
id: rendering-spatial-bridge
type: pattern
tags: [rendering, physics, bridge]
---
# Pattern: Rendering Spatial Bridge

## 1. Geometry
- **Requirement:** Implement a one-way mapping from the `Physics Domain` (simulation) to the `Rendering Domain` (display).
- **Requirement:** Maintain a scale factor (e.g., `Meters-to-Pixels`).
- **Rule:** Perform coordinate transformation (e.g., Y-axis flipping if engine and renderer coordinate systems differ).
- **Rule:** Perform unit conversion (e.g., internal radians to renderer degrees).
- **Rule:** Physics state must never be modified by the rendering layer.

## 2. Nuance
Decoupling the simulation scale from the screen scale allows for easier zooming, camera manipulation, and support for high-DPI displays without breaking the game's balance.

## 3. Verify
- The renderer queries the simulation state but does not write to it.
- Conversions are centralized in a single mapping function or system.
- Orientation and scale are applied consistently across all entities.
