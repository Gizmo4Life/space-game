# Module Standard: Logistics (Cargo)

Cargo modules are the backbone of the interstellar economy, providing the internal volume ($V$) required to transport commodities, ores, and manufactured goods.

## Volume Formula

The cargo volume ($V$) in cubic meters is calculated as:

$$V = \text{Base\_Volume} \times TM_{cargo}(\text{Volume\_Tier})$$

### Cargo Tier Multipliers ($TM_{cargo}$)

Unlike other module types, Cargo scales **linearly** to reflect the simpler, low-tech nature of open storage space.

| Tier | Multiplier |
| :--- | :--- |
| **T1** | $1.0$ |
| **T2** | $2.5$ |
| **T3** | $4.0$ |

## Performance Table

| Size Tier | Base Volume ($B$) | T1 Vol | T2 Vol | T3 Vol |
| :--- | :--- | :--- | :--- | :--- |
| **T1** | 50 m³ | 50 | 125 | 200 |
| **T2** | 150 m³ | 150 | 375 | 600 |
| **T3** | 400 m³ | 400 | 1,000 | 1,600 |

## Physical Attributes

Cargo modules are primarily empty space but require structural gating and localized inertial stabilization.

| Size Tier | Base Volume | Base Mass | Base Power Draw |
| :--- | :--- | :--- | :--- |
| **T1** | 10 $u^3$ | 20 t | 15 GW |
| **T2** | 30 $u^3$ | 60 t | 45 GW |
| **T3** | 80 $u^3$ | 160 t | 120 GW |

### Physical Reduction Multipliers ($RM$)
Higher attribute tiers reflect optimized bracing and stabilization, reducing the physical footprint:
- **T1**: $1.0$ (Baseline)
- **T2**: $0.9$ (Refined)
- **T3**: $0.75$ (Advanced)

---

*For universal scaling principles, see the [Module Performance Hub](../module-performance-standard.md).*
