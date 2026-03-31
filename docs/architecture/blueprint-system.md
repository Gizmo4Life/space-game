# Blueprint System

The blueprint system defines the static configuration of a ship's hull and its installed modules.

## Data Structure
- `ShipBlueprint`: Contains hull reference and module list.
- `HullDef`: Defines slots and physical characteristics.
- `ModuleDef`: Defines module attributes and requirements.

## Blueprint Generation (Genetic Algorithm)

To ensure high-quality and viable ship designs, the `ShipOutfitter` uses a Genetic Algorithm (GA) to evolve blueprints over multiple generations.

### Evolutionary Cycle
- **Population**: 16 candidates per request.
- **Generations**: 8 iterations (with early exit if fitness >= 80%).
- **Elitism**: The top 4 candidates from each generation are preserved unchanged in the next generation.
- **Selection**: Parents for crossover and mutation are selected from the top 8 candidates.

### Genetic Operators
- **Crossover (50% chance)**: Single-point crossover between two parents, swapping modules at a random index to combine successful traits.
- **Mutation (20% chance)**: Replaces a random module with a fresh, role-appropriate alternative to maintain diversity and discover new optimizations.
- **Correction**: Candidates that fail validation after genetic operations are replaced with fresh random candidates to ensure a valid population.

### Fitness Evaluation
Fitness is calculated based on role-specific metrics (Combat, Cargo, Transport) and normalized to a **0-100% scale** for consistent reporting in the UI and telemetry.
- **Combat**: Balanced for damage output, armor tier, and thrust-to-weight ratio.
- **Cargo/Transport**: Optimized for capacity and efficiency while maintaining minimum defensive standards.
