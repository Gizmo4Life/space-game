---
id: module-batteries
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](../readme.md) > [Module Performance](../module-performance-standard.md) > Batteries

# Module Standard: Energy Storage (Batteries)

Batteries store surplus energy from reactors and provide buffer capacity for high-draw systems like Weapons and Shields.

## Capacity Formula

The total capacity ($C$) in GJ is calculated as:

$$C = \text{Base\_Capacity} \times TM(\text{Capacity\_Tier})$$

### Tier Multipliers ($TM$)

| Tier | Multiplier |
| :--- | :--- |
| **T1** | $1.0$ |
| **T2** | $3.0$ |
| **T3** | $8.0$ |

## Performance Table

| Size Tier | Base Capacity ($B$) | T1 Cap | T2 Cap | T3 Cap |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | 5,000 GJ | 5,000 | 15,000 | 40,000 |
| **T2** | 15,000 GJ | 15,000 | 45,000 | 120,000 |
| **T3** | 40,000 GJ | 40,000 | 120,000 | 320,000 |

## Physical Attributes

Batteries scale their physical footprint based on the capacitor technology used.

| Size Tier | Base Volume | Base Mass | Base Power Draw |
| :--- | :--- | :--- | :--- |
| **T1** | 10 $u^3$ | 20 t | 15 GW |
| **T2** | 30 $u^3$ | 60 t | 45 GW |
| **T3** | 80 $u^3$ | 160 t | 120 GW |

### Physical Reduction Multipliers ($RM$)
Higher attribute tiers reflect capacitor optimization, reducing the physical footprint:
- **T1**: $1.0$ (Baseline)
- **T2**: $0.9$ (Refined)
- **T3**: $0.75$ (Advanced)

---

*For universal scaling principles, see the [Module Performance Hub](../module-performance-standard.md).*
