---
id: economy-refit-fee
type: pattern
tags: [economy, shipyard, credits]
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](/docs/developer/pattern/readme.md) > Economy Refit Fee

# Pattern: Economy Refit Fee

This pattern governs the installation costs associated with modifying ship modules at a faction-controlled shipyard.

## 1. Geometry
- **Requirement:** Charge an installation fee when a player installs a module on their ship.
- **Rule:** The fee is calculated as `Base Market Price + 50.0f` credits.
- **Rule:** Fees are paid directly to the faction owning the planetary economy.
- **Rule:** NPC entities are exempt from fees (simulated as internal maintenance).

## 2. Nuance
The refit fee prevents trivial hot-swapping and adds a "maintenance" feel to ship ownership. The fixed `50.0f` overhead represents labor and docking fees, while the module price component ensures that high-tier installations remain a significant economic decision.

## 3. Verify
- [ ] Swapping a T1 engine costs more than just the module stock price.
- [ ] The local faction's credit balance increases by the total refit fee.
- [ ] Attempting a refit with insufficient credits fails and leaves the ship unchanged.
