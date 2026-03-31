# Module Standard: Logistics (Habitation)

Habitation modules provide the life support, radiation shielding, and quarters necessary for crew and passenger transport.

## Capacity Formula

The passenger capacity ($Cap$) in standard adult units is calculated as:

$$Cap = \text{Base\_Capacity} \times TM(\text{Capacity\_Tier})$$

### Tier Multipliers ($TM$)

Habitation follows **Standard Module Scaling** due to the extreme complexity of life support systems.

| Tier | Multiplier |
| :--- | :--- |
| **T1** | $1.0$ |
| **T2** | $3.0$ |
| **T3** | $8.0$ |

## Performance Table

| Size Tier | Base Cap ($B$) | T1 Cap | T2 Cap | T3 Cap |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | 10 Adults | 10 | 30 | 80 |
| **T2** | 30 Adults | 30 | 90 | 240 |
| **T3** | 80 Adults | 80 | 240 | 640 |

> [!TIP]
> Habitation modules also provide food/water storage calculated at $10\%$ of their passenger capacity volume.

## Physical Attributes

| Size Tier | Base Volume | Base Mass | Base Power Draw |
| :--- | :--- | :--- | :--- |
| **T1** | 10 $u^3$ | 20 t | 15 GW |
| **T2** | 30 $u^3$ | 60 t | 45 GW |
| **T3** | 80 $u^3$ | 160 t | 120 GW |

### Physical Reduction Multipliers ($RM$)
Higher attribute tiers reflect technological optimization of life support and quarters:
- **T1**: $1.0$ (Baseline)
- **T2**: $0.9$ (Refined)
- **T3**: $0.75$ (Advanced)

---

*For universal scaling principles, see the [Module Performance Hub](../module-performance-standard.md).*
