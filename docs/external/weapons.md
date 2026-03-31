# Weapon Systems Guide

Understanding the various weapon archetypes and how their attribute tiers (T1, T2, T3) influence combat effectiveness is key to mastering ship outfitting.

## Weapon Archetypes

Combat is divided into three primary archetypes, each with distinct behaviors and ideal engagement envelopes. All weapon types have a range, and rate of fire attribute.

### 1. Energy Weapons (T1 - Beams)
Energy weapons provide instantaneous damage via high-intensity beams. They are ideal for high-precision strikes at short-to-medium range.

- **Mechanic**: Instant travel time (hitscan).
- **Core Attributes**:
    - **Range**: Higher tiers increase the maximum reach of the beam ($Range\_Tier \times 200.0$).
    - **Power**: Caliber tiers determine the continuous damage applied per tick.
    - **Efficiency**: Higher efficiency reduces the battery cost per shot.

> [!TIP]
> Use Energy weapons for fast-moving targets or when you absolutely need to land every shot.

### 2. Ballistic Weapons (T2 - Projectiles)
Ballistic weapons fire physics-based projectiles. Damage is derived from the mass and velocity of the round at the moment of impact.

- **Mechanic**: Physics-driven objects. Damage scales with the square of relative velocity.
- **Core Attributes**:
    - **Caliber**: Determines the mass of the projectile and its visual size. Larger rounds carry more kinetic energy.
    - **Muzzle Velocity**: Weapons with higher range tiers fire rounds at higher initial speeds ($Range\_Tier \times 1500.0$).
    - **Warheads**: Specialized ammo can provide Explosive or EMP effects on impact.

### 3. Missile Weapons (T3 - Self-Propelled)
Missiles are guided munitions that provide long-range, high-damage capabilities.

- **Mechanic**: Guidance-driven (Boid-Seek behavior). Missiles accelerate over time toward their target.
- **Core Attributes**:
    - **Acceleration**: Larger missiles (higher caliber tier) have more powerful thrusters.
    - **Guidance**: Tiers determine the tracking quality (T1: Unguided, T2: Heat-seeking, T3: Intelligent tracking).
    - **Payload**: Large Area of Effect (AOE) explosions. Damage falls off toward the edge of the blast radius.

---

## Attribute Tiers and Scaling

Every weapon and module in the game is rated by its **Tier (1, 2, or 3)**. These tiers are plugged into formulas that determine the final technical performance.

### Universal Scaling
Most attributes use a Tier Multiplier:
- **T1**: Baseline performance (1.0x).
- **T2**: Significant upgrade (1.5x).
- **T3**: Elite performance (2.0x).

### Formula Highlights
| Property | Archetype | Formula Mapping |
| :--- | :--- | :--- |
| **Beam Reach** | Energy | $200 \times Range\_Tier$ |
| **Kinetic Damage** | Ballistic | $0.5 \times Mass \times (V_{rel} / 100)^2$ |
| **Missile Accel** | Missile | $10 + (Caliber\_Tier \times 15)$ |

---

## Technical Appendix: Ammo Variants

Ammunition choice is just as important as the weapon itself.

| Warhead Type | Effect | Ideal Use |
| :--- | :--- | :--- |
| **Solid** | Kinetic impact | Hull penetration. |
| **Explosive** | Area damage | Clustered enemies or components. |
| **EMP** | System disable | Disabling engines/weapons without destroying the ship. |

> [!NOTE]
> Module performance is fully deterministic — two weapons of the same tier and archetype always perform identically. Variety comes from the combination of different attribute tiers across your loadout.
