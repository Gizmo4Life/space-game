---
id: render-mode-dispatch
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Render Mode Dispatch

# Pattern: Render Mode Dispatch

An enum-driven conditional branch within a rendering function that selects between two or more visual representations of the same entity data. The dispatch point is at the renderer level, not the caller.

## Structure

1. A **mode enum** defines the available visual styles (e.g., `Game`, `Schematic`, `Debug`).
2. A **params struct** carries the mode alongside other rendering parameters (scale, color, rotation).
3. The **renderer function** checks the mode and branches to the appropriate drawing path.
4. Callers set the mode in the params; they do not implement mode-specific logic.
