---
id: proxy-identity-overlap
type: pattern
pillar: developer
---

# Pattern: Proxy Identity Overlap (Avoided Guards)

## Context
Systems often use high-level "proxy" identities like Factions or Relationship IDs to determine logic branches (e.g., "Allies don't pay for fuel"). In tests, if you default the player and an NPC to the same faction, you might trigger these "free" branches unintentionally, bypassing the specific logic you wanted to test (like transaction balance).

## Preferred (P)
**Use distinct identities for interacting entities in tests unless you specifically want to test the shared-identity logic.** Be explicit about why entities share an identity.

```cpp
// Preferred: Distinct factions to test transaction logic
auto player = registry.create();
registry.emplace<FactionComponent>(player, Faction::Player);

auto station = registry.create();
registry.emplace<FactionComponent>(station, Faction::Civilian);

// Now transaction logic will actually run the "payment" branch
MarketSystem::purchase(registry, player, station, item);
```

## Unacceptable (U)
**Silent identity overlap.** Letting entities default to the same faction/ID, causing logic to skip important steps (like charging money) without the developer noticing.

```cpp
// Unacceptable: Both default (e.g., Faction::None or shared initial value)
auto player = registry.create();
auto station = registry.create();

MarketSystem::purchase(registry, player, station, item);
// Logic might see "Same Faction" and grant the item for free!
// Test passes "Item received", but "Money deducted" was never even attempted.
```

## Nuance
Characterize your test entities as "The Buyer" and "The Seller" and ensure their relationship state is explicitly set to the state you intend to test (Hostile, Allied, Neutral).
