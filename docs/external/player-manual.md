---
id: player-manual
type: guide
pillar: external
---
[Home](/) > [Docs](/docs/readme.md) > [External](/docs/external/readme.md) > Player Manual

# Player Manual: Fleet Logistics & Resource Management

Welcome, Commander. This guide covers the essential systems for managing your fleet's survival and operational efficiency in deep space.

## 1. Core Resources
Your vessels depend on four primary resources. Depletion of any of these will lead to mission failure or vessel loss.

| Resource | Icon | Purpose | Warning Signs |
| :--- | :--- | :--- | :--- |
| **Fuel** | `F` | Required for all propulsion and maneuvering. | Vessel becomes immobile at 0%. |
| **Food** | `Fd` | Sustains your crew and passengers. | Starvation leads to rapid population loss. |
| **Isotopes** | `I` | Reactor fuel for power generation. | Surpluses are stored in batteries; 0% stops charging. |
| **Ammo** | `A` | Physical munitions for projectile and missile weapons. | Weapons will fail to cycle when dry. |

## 2. Interpreting the Fleet HUD
The **Fleet Overlay** (Top Right) provides real-time logistics data for every vessel under your command.

### Time to Exhaustion (TTE)
The most critical metric is **TTE**. This calculates how many seconds (hours/minutes) remain until your *most critical* resource runs out based on current consumption rates.
- **Stable**: Consumption is balanced or positive.
- **Red Alert**: TTE < 10 minutes. Immediate resupply required.

### Quantity Indicators
Displayed as `Current / Max`:
- `F: 50/100`: Half fuel remains.
- `A: 200/500`: Magazine at 40% capacity.

## 3. Logistics Strategy
- **Habitation Efficiency**: Tier 3 Habitation modules are significantly more efficient. Moving population to high-tier vessels reduces global food consumption.
- **Mass Impact**: Cargo and fuel add "Wet Mass." A fully loaded freighter will accelerate slower than an empty one.
- **Resupply**: Visit friendly orbital stations or planetary hubs to replenish stocks.

## 4. Vessel Outfitting & Shipyard
Planetary hubs offer services to refit your vessel or expand your fleet.

### The Outfitter
Accessible via the **[Outfitter]** tab when landed.
- **Consumption Analysis**: View your vessel's active power draw and resource consumption rates (GW/hour).
- **Module Compatibility**: Volumetric and mass limits apply according to your hull's specifications.

### The Shipyard
Accessible via the **[Shipyard]** tab.
- **Fleet Management**: Cycle through your owned vessels to view status or sell ships at a market-adjusted rate.
- **Ship Quality**: All ships available for purchase meet a minimum **fitness score of 50%** for their designated role and come pre-provisioned with at least **5 days' worth** of food, fuel, and isotopes.
- **Cargo Preservation**: When purchasing a new flagship, your existing cargo is automatically transferred to the new vessel.

### UI Stability & Data Availability
During complex operations (such as refitting engines or transferring ships between fleets), a vessel may briefly report **"Vessel data temporarily unavailable"** in the UI. This is a safety measure to ensure system stability while the ship's physical configuration is being recalculated.

### Reequip for Duration
Accessible via the **[5] Reequip** tab when landed.
- **Duration Selection**: Use Up/Down arrows to set a target duration (1-30 days).
- **Fleet-Wide**: The calculation aggregates consumption and cargo across **all ships in your fleet** (flagship + escorts), so every vessel gets provisioned.
- **Auto-Purchase**: Press **Enter** to buy food, fuel, and isotopes up to the specified duration, constrained by credits, cargo capacity, and market supply.
- **Preview**: Before purchasing, the panel shows required quantities, unit prices, market stock, and estimated costs.

---
> [!IMPORTANT]
> Always verify your fleet's TTE before initiating long-range jumps or entering hostile sectors.
