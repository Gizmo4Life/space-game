---
id: span-economy-stockpile-imbalance
type: span_runbook
module: game-economy-module
pillar: operational
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](readme.md) > economy.stockpile.imbalance

# Span Runbook: Stockpile Imbalance

## 1. Component Scope
- **Module:** [Game Economy](/docs/architecture/module/game-economy.md)
- **Signal:** High `game.economy.stockpile.delta` but zero `game.economy.ship_assembly` throughput.
- **Description:** Tracks "Resource Starvation" where factions have hulls but lack critical modules (Engines/Reactors) to finish ships.

## 2. Diagnostic Mapping
| Metric | Threshold | logic |
| :--- | :--- | :--- |
| **Price Delta** | > 500% | Specific module price (e.g., T1 Engine) ballooning due to zero supply. |
| **Assembly Gap** | > 5 min | Scrapyard has hulls but no assembly spans recorded. |

**Isolate Failure:**
- Check `game.economy.stockpile.delta` for `Resource::Metals` or `Resource::Electronics`.
- Compare `faction.credits` against building costs; if low, the faction can't expand its factories.

## 3. Mitigation & Restoration
1. **Seed Stockpile:** Manually inject `Resource::Isotopes` or `Resource::Metals` into the planet's primary faction stockpile.
2. **Force Assemble:** Use developer console to trigger `tryAssembleShips` with debug bypass of module requirements.
3. **Credit Infusion:** Increase `FactionEconomy.credits` for the starving faction to allow infrastructure expansion.

## 4. Recovery Verification
- `game.economy.ship_assembly` span firing with `size_tier` matching the backlog.
- Module prices stabilize within 20% of base.
