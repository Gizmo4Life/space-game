---
id: navigation-capability
type: capability
pillar: architecture
---
# Capability: Navigation

## 1. Business Intent
Provide a realistic yet accessible flight model for spacecraft, allowing players to traverse solar systems and travel between them using jump links. This capability is the primary method of exploration and spatial positioning.

## 2. Orchestration Flow
1. **Input Handling:** Process raw player input for thrust and rotation via specialized input modules.
2. **Physics Simulation:** Apply forces to the ship entity using the [Physics] (T3) module. Navigation operates on the `SHIP_SCALE` (30.0) for combat and ship-level movement, while using `WORLD_SCALE` (0.05) for solar system-level positioning and orbital mechanics.
3. **Alignment**: Thrust and rotation are aligned to the ship's +X forward axis, ensuring intuitive traversal.
4. **Link Transit:** Detect proximity to [SystemGate] (T3) entities to trigger interplanetary transitions.

## 3. Data Flow & Integrity
- **Trigger:** Continuous player input or autopilot commands.
- **Output:** Updated world coordinates and velocity vectors for the player ship.
- **Consistency:** Eventually consistent spatial sync across client/server (if applicable), but locally authoritative for responsiveness.

## 4. Operational Context
- **Primary Runbook:** [Universal System Runbook](/docs/operational/runbook/universal-runbook.md)
- **Critical Failure Metric:** Velocity jitter or "clipping" through boundaries exceeding 5% of frames.
