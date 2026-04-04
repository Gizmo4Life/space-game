---
id: module-reactors
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](../readme.md) > [Module Performance](../module-performance-standard.md) > Reactors

# Module Standard: Power Generation (Reactors)

Reactors provide the baseline energy required for all ship systems. They consume isotopes to generate GW-scale power.

## Output Formula

The power output ($O$) in GW is calculated as:

$$O = \text{Base\_Output} \times TM(\text{Output\_Tier})$$

### Tier Multipliers ($TM$)

| Tier | Multiplier |
| :--- | :--- |
| **T1** | $1.0$ |
| **T2** | $3.0$ |
| **T3** | $8.0$ |

## Performance Table

| Size Tier | Base Output ($B$) | T1 Output | T2 Output | T3 Output |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | 100 GW | 100 | 300 | 800 |
| **T2** | 300 GW | 300 | 900 | 2,400 |
| **T3** | 800 GW | 800 | 2,400 | 6,400 |

## Physical Attributes

Reactors are the densest modules on a ship. Higher tiers provide better shielding (Mass) and magnetic containment (Volume).

| Size Tier | Base Volume | Base Mass | Base Output (Efficiency=T1) |
| :--- | :--- | :--- | :--- |
| **T1** | 10 $u^3$ | 20 t | 100 GW |
| **T2** | 30 $u^3$ | 60 t | 300 GW |
| **T3** | 80 $u^3$ | 160 t | 800 GW |

### Physical Reduction Multipliers ($RM$)
Higher attribute tiers reduce reactor shielding mass and containment volume:
- **T1**: $1.0$ (Baseline)
- **T2**: $0.9$ (Refined)
- **T3**: $0.75$ (Advanced)

> [!TIP]
> Efficiency tiers for reactors **reduce mass** while **increasing output** proportionally beyond the base $(Output_{final} = Output_{base} \times TM_{eff})$.

---

*For universal scaling principles, see the [Module Performance Hub](../module-performance-standard.md).*
