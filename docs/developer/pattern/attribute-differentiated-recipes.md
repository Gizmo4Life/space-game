# Attribute-Differentiated Recipes (ADR)

## Context
In a procedural universe, items often have randomized or tiered attributes (e.g., Thrust, Efficiency, Range). A static production cost per item type fails to reflect the true complexity and "worth" of these variants.

## Pattern
The **ADR** pattern links individual item attributes to specific resource consumption inputs during the production process.

### Implementation
1. **Attribute Mapping**: Each `AttributeType` is associated with one or more `Resource` types (e.g., `Thrust` -> `Metals`, `Efficiency` -> `Isotopes`).
2. **Tier Scaling**: The amount of resource required scales with the attribute's tier/star rating (e.g., 3 resources for 1*, 5 for 2*, 8 for 3*).
3. **Total Quality Penalty**: An additional overhead is applied based on the total complexity of the item (e.g., +1 of each used resource for every 4 total stars).
4. **Final Magnitude Scaling**: The entire resource set is multiplied by a magnitude factor (Size Tier) to account for different scales of items (Small vs. Capital).

## Consequences
- **Economic Realism**: Rare/High-quality items are harder to mass-produce, requiring specialized supply chains.
- **Dynamic Scarcity**: A faction with plenty of `Metals` but no `Rare Metals` can build basic engines but will struggle to produce high-thrust variants.
- **Design Incentive**: Players can see the "cost" of quality, making salvaged high-tier modules more valuable.
