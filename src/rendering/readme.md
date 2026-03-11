# Rendering Module

## Overview
This directory contains classes and utilities responsible for the visual representation of the game world, including UI panels, ship rendering, and scene management.

## Key Components
- **ShipRenderer**: Handles procedural drawing of ship hulls and modules.
- **ShipyardPanel**: UI for purchasing and selling vessels, featuring a two-pane layout with automatic scrolling.
- **OutfitterPanel**: UI for managing ship modules and ammunition, featuring a three-column layout with scrollable lists and a detail pane.
- **LandingPanel**: Base class for planetside UI screens.
- **UIUtils**: Common helper functions for formatting and UI drawing.

## UI Navigation Standards
- **Vertical Navigation**: `W`/`S` or Arrow keys.
- **Scrolling**: Lists scroll automatically based on selection.
- **Detail Panes**: Use `[` and `]` for vertical scrolling.
- **Tab Switching**: Use `Tab` or `Z` where appropriate.

## Documentation
See [docs/architecture/module/rendering.md](/docs/architecture/module/rendering.md) for detailed architectural patterns.
