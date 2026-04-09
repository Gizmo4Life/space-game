---
id: mission-accounting
type: pattern
pillar: developer
---
[Home](/) > [Docs](/docs/readme.md) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Mission Accounting

# Pattern: Mission Accounting

**Context:** Verifying mortality and mission success for NPCs during automated accounting or unit testing.

**Pattern:**
1. Spawn ship via `NPCShipManager`.
2. Mock an active mission in `inst.activeMissions_`.
3. Call `NPCShipManager::recordCombatDeath`.
4. Verify `attackerFaction.stats.outfitRegistry` reflects the kill.

**Constraint:** `recordCombatDeath` does not handle entity destruction; this must be handled by the calling system (e.g., `WeaponSystem`).
