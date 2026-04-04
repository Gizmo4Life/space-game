---
id: economy-transaction-runbook
type: span_runbook
module: game-economy-module
---
[Home](/) > [Docs](/docs/readme.md) > [Operational](/docs/operational/readme.md) > [Span](/docs/operational/span/readme.md) > Economy Transaction

# Span Runbook: game.economy.transaction

## 1. Component Scope
- **Module**: [Game Economy](/docs/architecture/module/game-economy.md)
- **Primary Source**: `EconomyManager::executeTrade` (lines 1100-1237)
- **Pillar**: Architecture / Game Logic

## 2. Diagnostic Mapping
| Observation | Logic Mode | Isolation |
| :--- | :--- | :--- |
| `fleet_size` is always 1 | Single-ship mode | Verify if escorts have `NPCComponent::isPlayerFleet` set to true. |
| `quantity` is partial | Buy Overflow | Check `CargoComponent::maxCapacity` on all fleet members. |
| Span fails to emit | Entry Bypass | Verify `registry.valid(planet)` and `registry.valid(player)` at transaction start. |

## 3. Mitigation & Restoration
1. **Fleet Desync**: If resources are charged but not distributed, use the `Fleet Reset` command to re-read registry relationships.
2. **Ghost Transactions**: If `credits` are deducted but `cargo` is unchanged, check for concurrent `add()` failures in `EconomyManager.cpp:1117`.

## 4. Recovery Verification
- **Probe**: `game.economy.transaction` emits with `fleet_size > 1` during a bulk buy at any planetary market.
