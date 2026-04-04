---
id: module-avionics
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](../readme.md) > [Module Performance](../module-performance-standard.md) > Avionics

# Module Standard: Avionics (Reaction Wheels)

Reaction wheels provide the rotational torque required to reorient the ship. Unlike engines, they consume only electrical power.

## Turn Torque Formula

The rotational torque ($Q$) in Newton-meters is calculated as:

$$Q = \text{Base\_Torque} \times TM(\text{TurnRate\_Tier})$$

### Tier Multipliers ($TM$)

| Tier | Multiplier |
| :--- | :--- |
| **T1** | $1.0$ |
| **T2** | $3.0$ |
| **T3** | $8.0$ |

## Performance Table

| Size Tier | Base Torque ($B$) | T1 Output | T2 Output | T3 Output |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | 2,000 Nm | 2,000 | 6,000 | 16,000 |
| **T2** | 6,000 Nm | 6,000 | 18,000 | 48,000 |
| **T3** | 16,000 Nm | 16,000 | 48,000 | 128,000 |

## Physical Attributes

Reaction wheels are heavy, high-speed inertial systems.

| Size Tier | Base Volume | Base Mass | Base Power Draw |
| :--- | :--- | :--- | :--- |
| **T1** | 10 $u^3$ | 20 t | 15 GW |
| **T2** | 30 $u^3$ | 60 t | 45 GW |
| **T3** | 80 $u^3$ | 160 t | 120 GW |

### Physical Reduction Multipliers ($RM$)
Higher attribute tiers reduce the inertial mass and motor volume required for torque:
- **T1**: $1.0$ (Baseline)
- **T2**: $0.9$ (Refined)
- **T3**: $0.75$ (Advanced)

---

*For universal scaling principles, see the [Module Performance Hub](../module-performance-standard.md).*
