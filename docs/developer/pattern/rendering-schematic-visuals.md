---
id: rendering-schematic-visuals
type: pattern
pillar: developer
category: ux
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Rendering Schematic Visuals

# Pattern: Rendering Schematic Visuals

A rendering mode that displays entities as wireframe outlines or blueprint-style schematics rather than filled polygons, conveying structural information without textures.

## Structure

1. Hull polygon vertices are drawn as a **line loop** (outline only, no fill).
2. Internal features (slots, module regions) are rendered as **dashed or thin lines** within the outline.
3. Color is derived from a categorical attribute (e.g., faction, role), using a reduced-opacity palette.
4. The rendering path is a conditional branch from the standard filled-polygon renderer.
