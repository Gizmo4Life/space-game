---
id: npc-fleet-leader-boids
type: pattern
polarity: prescriptive
pillar: developer
---
[Home](/) > [Developer](/docs/developer/readme.md) > [Pattern](readme.md) > Fleet Leader Boids

# Pattern: Fleet Leader Boids

**Intent:** Allow a group of NPC ships to follow a designated leader entity using weighted boids steering, with aggressive re-cohesion at large distances.

## Shape

```cpp
struct NPCComponent {
  // ... standard fields ...
  bool         isPlayerFleet = false;
  entt::entity leaderEntity  = entt::null;
};

// In NPCShipManager::spawnShip — on purchase:
npc.isPlayerFleet = true;
npc.leaderEntity  = playerEntity;
npc.belief        = AIBelief::Escort;
npc.targetEntity  = playerEntity;

// In NPCShipManager::tickAI — boids pass:
bool isLeader = (other == npc.leaderEntity);
float weight  = isLeader ? 4.0f : 1.0f;     // leader has 4× cohesion influence
avgPos += oPos * weight;
neighborCount += weight;

// Aggressive follow-back when far from leader:
if (npc.isPlayerFleet && ldSq > 200.0f * 200.0f) {
  float tf = inertial.thrustForce;
  b2Body_ApplyForceToCenter(bodyId, {ldx/ld * tf * 3.0f, ldy/ld * tf * 3.0f}, true);
}
```

## Key Constraints
- **Leader weight** — Leader cohesion/alignment is 4× that of regular allies.
- **Aggressive catch-up threshold** — 200 world units; applies 3× `thrustForce` toward leader.
- **Separation still applies globally** — Fleet ships avoid all nearby ships (not just allies).
- **Escort belief** — Fleet ships are set to `AIBelief::Escort` targeting the leader; they participate in combat against the leader's enemies.
- **Entity validity** — Always guard with `registry.valid(npc.leaderEntity)` before dereferencing.

## Applied In
- `NPCComponent` — `isPlayerFleet`, `leaderEntity` fields.
- `NPCShipManager::spawnShip` — initialization when `isPlayerFleet = true`.
- `NPCShipManager::tickAI` — leader-weighted boids and distance-check aggressive thrust.
