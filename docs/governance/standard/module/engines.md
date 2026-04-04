---
id: module-engines
type: standard
pillar: governance
---
[Home](/) > [Docs](/docs/readme.md) > [Governance](/docs/governance/readme.md) > [Standard](../readme.md) > [Module Performance](../module-performance-standard.md) > Engines

# Module Standard: Propulsion (Engines)

Engines translate electrical power into kinetic thrust. Performance follows a strict tiered scaling to ensure predictable ship acceleration profiles.

## Thrust Formula

The final thrust ($T$) in Newtons is calculated as:

$$T = \text{Base\_Thrust} \times TM(\text{Thrust\_Tier})$$

### Tier Multipliers ($TM$)

| Tier | Multiplier |
| :--- | :--- |
| **T1** | $1.0$ |
| **T2** | $3.0$ |
| **T3** | $8.0$ |

## Performance Table

| Size Tier | Base Thrust ($B$) | T1 Output | T2 Output | T3 Output |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | 8,000 N | 8,000 | 24,000 | 64,000 |
| **T2** | 24,000 N | 24,000 | 72,000 | 192,000 |
| **T3** | 64,000 N | 64,000 | 192,000 | 512,000 |

> [!NOTE]
> Physical properties (Mass, Volume) are determined by the **Size Tier**, while functional output is determined by the **Thrust Tier**.

## Physical Attributes

Engines define their physical footprint based on the **Size Tier**, with higher **Mass** and **Volume** tiers providing technological reductions.

| Size Tier | Base Volume | Base Mass | Base Power Draw |
| :--- | :--- | :--- | :--- |
| **T1** | 10 $u^3$ | 20 t | 15 GW |
| **T2** | 30 $u^3$ | 60 t | 45 GW |
| **T3** | 80 $u^3$ | 160 t | 120 GW |

### Physical Reduction Multipliers ($RM$)
- **T1**: $1.0$ (Baseline)
- **T2**: $0.9$ (Refined)
- **T3**: $0.75$ (Advanced)

$$V_{final} = V_{base} \times RM(\text{Volume\_Tier}), \quad M_{final} = M_{base} \times RM(\text{Mass\_Tier})$$
