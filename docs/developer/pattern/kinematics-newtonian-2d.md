---
id: kinematics-newtonian-2d
type: pattern
tags: [physics, kinematics, math]
---
# Pattern: Newtonian Kinematics (2D)

## 1. Geometry
- **Requirement:** Implement a "Velocity Verlet" or similar integration step.
- **Requirement:** Store state as `Position` (vector), `Velocity` (vector), and `Orientation` (scalar).
- **Rule:** Apply `Thrust` as a force vector derived from `Orientation` along the ship's +X forward axis.
- **Rule:** Implement `Inertia` by not zeroing velocity between frames.
- **Rule:** Implement `Dual-Scale Rendering` by using a separate `SHIP_SCALE` for ship physics/visuals to ensure visibility in a vast world.
- **Rule:** (Optional) Apply `Damping` or `Drag` as a percentage of current velocity to simulate atmospheric resistance or spatial friction.

## 2. Nuance
The "Escape Velocity" feel relies on the interplay between instant rotational adjustments and slow, inertial translational changes.

## 3. Verify
- Displacement is calculated as `pos += vel * dt`.
- Forces are accumulated into an `Acceleration` vector before being applied to `Velocity`.
