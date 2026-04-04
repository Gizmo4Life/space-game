---
id: boarding-protocol
type: pattern
pillar: architecture
category: engine
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Pattern: Boarding Protocol
# Pattern: Boarding Protocol

Manage stateful interactions for resource transfer and fleet acquisition between vessels.

## 1. Problem
Interaction between two vessels (e.g., player ship and derelict) requires complex state synchronization for bidirectional resource flow and secondary actions like scuttling or joining a fleet.

## 2. Solution
Implement a `BoardingComponent` to track the link and a `BoardingSystem` to orchestrate discrete commands. Mandatory safety checks must prevent boarding of hostile, active targets.

## 3. Constraints
- **Target State**: Target must be `isDerelict` (Unstaffed or EMP-incapacitated) or `isFriendly`.
- **Proximity**: Entities must maintain a specific collision/overlap state or distance.
- **Resource Clamping**: Transfers must never exceed source availability or target capacity.

## 4. Operational Actions
- **Transfer**: Power, Fuel, Ammo, Cargo.
- **Fleet Alignment**: Change `Faction` and restore `isStaffed` state.
- **Destruction**: Scuttle command for permanent entity removal.

## 5. Implementation
See `BoardingSystem.h/.cpp` for the reference implementation of the static command interface.
