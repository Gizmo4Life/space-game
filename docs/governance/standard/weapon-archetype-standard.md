# Standard: Weapon & Ammo Archetypes

This standard defines how abstract **Attribute Tiers** (T1, T2, T3) are mapped to technical implementation values for the three primary weapon archetypes using deterministic scaling.

## 1. Unified Scaling Logic

All weapon performance metrics follow the **Standard Tier Multiplier (TM)**:
- **TM(t)**: T1 = 1.0, T2 = 3.0, T3 = 8.0.
- **Deterministic**: Random variance (Quality Rolls) is removed. All modules of the same tier have identical performance for the same attributes.

## 2. Physical Optimization

Higher-tier weapons use advanced materials and integrated cooling to reduce their physical footprint:
- **T1 (Basic)**: 1.0 (Baseline)
- **T2 (Refined)**: 0.9 (10% reduction)
- **T3 (Advanced)**: 0.75 (25% reduction)

This optimization applies to **Mass**, **Volume**, and internal **Power Draw**.

## 3. Energy Weapons (T1 - Beams)

Energy weapons provide instantaneous damage via line-trace raycasting. Ideal for high-precision, short-to-medium range engagement.

| Technical Property | Formula | Baseline (T1) |
| :--- | :--- | :--- |
| **Max Range** | `200.0 * TM(Range_Tier)` | 200.0 u |
| **Fire Cooldown** | `0.8 / TM(ROF_Tier)` | 0.8 s |
| **Energy Cost** | `(10.0 * TM(Caliber_Tier)) * (1.1 - Efficiency_Tier * 0.1)` | 10.0 GW |
| **Damage/Tick** | `5.0 * TM(Caliber_Tier)` | 5.0 dmg |
| **Beam Width** | `1.0 + (Size_Tier - 1) * 1.5` | 1.0 px |

## 4. Ballistic Weapons (T2 - Projectiles)

Ballistics fire physics-driven projectiles. Damage is calculated based on impact momentum.

| Technical Property | Formula | Baseline (T1) |
| :--- | :--- | :--- |
| **Muzzle Velocity** | `1500.0 * TM(Range_Tier)` | 1500.0 u/s |
| **Projectile Mass** | `2.0 * TM(Caliber_Tier)` | 2.0 kg |
| **Time to Live (TTL)**| `2.0 s` (Standardized) | 2.0 s |
| **Visual Scale** | `0.5 + (Caliber_Tier - 1) * 0.75` | 0.5x |
| **Impact Damage** | `0.5 * Mass * (V_relative / 100)^2` | Physics-based |

## 5. Missile Weapons (T3 - Self-Propelled)

Missiles are guided munitions with high-yield payloads and complex guidance systems.

| Technical Property | Formula | Baseline (T1) |
| :--- | :--- | :--- |
| **Acceleration** | `10.0 + TM(Caliber_Tier) * 15.0` | 25.0 u/s² |
| **Max Speed** | `20.0 + TM(Guidance_Tier) * 10.0` | 30.0 u/s |
| **Turn Rate** | `1.5 + TM(Guidance_Tier) * 2.0` | 3.5 rad/s |
| **Time to Live (TTL)**| `3.0 + TM(Guidance_Tier) * 2.0` | 5.0 s |
| **Explosion Radius** | `10.0 + Mass * 2.0` | 12.0 u |

- **Guidance Logic (Boid Seek)**: 
  - Missiles use force-based steering: `Steer = (DesiredVelocity - Velocity)`.
  - T1: Dumbfire (Forward thrust only).
  - T2: Heatseeking (Standard track closest target).
  - T3: Remote (High-precision tracking).

## 6. Implementation Rules
- **Encapsulation**: These formulas are centralized in `WeaponSystem::fire` to ensure balance consistency.
- **Telemetry**: Shot properties (length, mass, velocity) are logged to the `game.combat.fire` span.
- **All archetypes** must support **Range** and **ROF** attribute scaling.
