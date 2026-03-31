#include "engine/combat/WeaponSystem.h"
#include "game/components/AmmoComponent.h"
#include "game/components/InertialBody.h"
#include "game/components/ShipStats.h"
#include "game/components/TransformComponent.h"
#include "game/components/WeaponComponent.h"
#include "engine/telemetry/Telemetry.h"
#include "game/NPCShipManager.h"
#include <box2d/box2d.h>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace space {

void WeaponSystem::update(::entt::registry &registry, b2WorldId worldId, float dt) {
  auto span = Telemetry::instance().tracer()->StartSpan("combat.system.update");

  // 1. Update Cooldowns
  auto weaponView = registry.template view<WeaponComponent>();
  for (auto entity : weaponView) {
    auto &weapon = weaponView.get<WeaponComponent>(entity);
    if (weapon.currentCooldown > 0) {
      weapon.currentCooldown -= dt;
    }
  }

  // 2. Update Beams
  auto beamView = registry.template view<BeamComponent>();
  for (auto entity : beamView) {
    auto &beam = beamView.get<BeamComponent>(entity);
    beam.duration -= dt;
    if (beam.duration <= 0) {
      registry.destroy(entity);
    }
  }

  // 3. Update Explosions (AOE Damage)
  auto expView = registry.template view<ExplosionComponent>();
  for (auto entity : expView) {
    auto &exp = expView.get<ExplosionComponent>(entity);
    
    // Trigger AOE on first frame
    if (exp.duration == exp.maxDuration) {
       b2Vec2 pos = {exp.position.x, exp.position.y};
       
       auto shipView = registry.template view<ShipStats, InertialBody>();
       for (auto shipEnt : shipView) {
         auto &inertial = shipView.get<InertialBody>(shipEnt);
         b2Vec2 sPos = b2Body_GetPosition(inertial.bodyId);
         float dx = pos.x - sPos.x;
         float dy = pos.y - sPos.y;
         float distSq = dx*dx + dy*dy;
         float rSq = exp.radius * exp.radius;
         
         if (distSq <= rSq) {
           float dist = std::sqrt(distSq);
           auto &stats = shipView.get<ShipStats>(shipEnt);
           if (exp.isEmp) {
             stats.empTimer = std::max(stats.empTimer, 60.0f);
           } else {
             // Falloff damage: 1.0 at center, 0.0 at edge
             float damage = exp.damage * (1.0f - (dist / exp.radius));
             stats.currentHull -= damage;
             if (stats.currentHull <= 0) {
               NPCShipManager::recordCombatDeath(registry, shipEnt, exp.owner);
             }
           }
         }
       }
    }

    exp.duration -= dt;
    if (exp.duration <= 0) {
      registry.destroy(entity);
    }
  }

  // 4. Update Missiles (Boid Seek Guidance)
  auto missileView = registry.template view<MissileComponent, ProjectileComponent, InertialBody>();
  for (auto entity : missileView) {
    auto &missile = missileView.get<MissileComponent>(entity);
    auto &proj = missileView.get<ProjectileComponent>(entity);
    auto &inertial = missileView.get<InertialBody>(entity);
    
    if (!b2Body_IsValid(inertial.bodyId)) continue;
    
    b2Vec2 pos = b2Body_GetPosition(inertial.bodyId);
    b2Vec2 vel = b2Body_GetLinearVelocity(inertial.bodyId);

    // Guidance Logic
    if (missile.isHeatSeeking || missile.isRemote) {
        if (!registry.valid(missile.target)) {
            float minDistSq = 1000.0f * 1000.0f; // 1000 units seek range
            auto targets = registry.template view<ShipStats, InertialBody>();
            for (auto tEnt : targets) {
                if (tEnt == proj.owner) continue;
                auto &tInertial = targets.get<InertialBody>(tEnt);
                b2Vec2 tPos = b2Body_GetPosition(tInertial.bodyId);
                float dSq = (pos.x - tPos.x)*(pos.x - tPos.x) + (pos.y - tPos.y)*(pos.y - tPos.y);
                if (dSq < minDistSq) {
                    minDistSq = dSq;
                    missile.target = tEnt;
                }
            }
        }

        if (registry.valid(missile.target)) {
            auto* targetInertial = registry.try_get<InertialBody>(missile.target);
            if (targetInertial) {
                b2Vec2 tPos = b2Body_GetPosition(targetInertial->bodyId);
                b2Vec2 toTarget = {tPos.x - pos.x, tPos.y - pos.y};
                float dist = std::sqrt(toTarget.x*toTarget.x + toTarget.y*toTarget.y);
                
                if (dist > 0.1f) {
                    // Boid Seek: Steer = (Desired - CurrentVelocity)
                    b2Vec2 desiredVel = { (toTarget.x / dist) * missile.maxSpeed, (toTarget.y / dist) * missile.maxSpeed };
                    b2Vec2 steer = { desiredVel.x - vel.x, desiredVel.y - vel.y };
                    
                    // Limit steering force by acceleration
                    float steerLen = std::sqrt(steer.x*steer.x + steer.y*steer.y);
                    if (steerLen > missile.acceleration) {
                        steer = { (steer.x / steerLen) * missile.acceleration, (steer.y / steerLen) * missile.acceleration };
                    }
                    b2Body_ApplyForceToCenter(inertial.bodyId, steer, true);
                }
            }
        } else {
            // Dumb forward thrust if no target
            float angle = std::atan2(vel.y, vel.x);
            b2Vec2 thrust = {std::cos(angle) * missile.acceleration, std::sin(angle) * missile.acceleration};
            b2Body_ApplyForceToCenter(inertial.bodyId, thrust, true);
        }
    } else {
        // Dumb forward thrust
        float angle = std::atan2(vel.y, vel.x);
        b2Vec2 thrust = {std::cos(angle) * missile.acceleration, std::sin(angle) * missile.acceleration};
        b2Body_ApplyForceToCenter(inertial.bodyId, thrust, true);
    }
    
    // Clamp Speed
    b2Vec2 currentVel = b2Body_GetLinearVelocity(inertial.bodyId);
    float speed = std::sqrt(currentVel.x * currentVel.x + currentVel.y * currentVel.y);
    if (speed > missile.maxSpeed) {
        b2Vec2 clampedVel = { (currentVel.x / speed) * missile.maxSpeed, (currentVel.y / speed) * missile.maxSpeed };
        b2Body_SetLinearVelocity(inertial.bodyId, clampedVel);
        speed = missile.maxSpeed;
    }
    
    // Face velocity direction
    if (speed > 0.1f) {
        float faceAngle = std::atan2(currentVel.y, currentVel.x);
        b2Body_SetTransform(inertial.bodyId, pos, b2MakeRot(faceAngle));
    }
  }

  // 5. Update Projectiles (TTL)
  auto projView = registry.template view<ProjectileComponent, InertialBody>();
  for (auto entity : projView) {
    auto &proj = projView.get<ProjectileComponent>(entity);
    proj.timeToLive -= dt;
    if (proj.timeToLive <= 0) {
      if (proj.isExplosive || proj.isEmp) {
        // Spawn explosion at end of life
        auto explosion = registry.create();
        b2Vec2 pos = b2Body_GetPosition(registry.get<InertialBody>(entity).bodyId);
        registry.emplace<ExplosionComponent>(explosion, 
            sf::Vector2f(pos.x, pos.y),
            10.0f + proj.mass * 2.0f,
            proj.damage,
            proj.isEmp, 0.5f, 0.5f, proj.owner
        );
        registry.emplace<TransformComponent>(explosion, sf::Vector2f(pos.x * 30.0f, pos.y * 30.0f));
      }
      b2DestroyBody(registry.get<InertialBody>(entity).bodyId);
      registry.destroy(entity);
      continue;
    }

    // Proximity Collision Check (Test 37 requirement)
    b2Vec2 pPos = b2Body_GetPosition(registry.get<InertialBody>(entity).bodyId);
    auto shipView = registry.view<ShipStats, InertialBody>();
    for (auto shipEnt : shipView) {
        if (!registry.valid(shipEnt) || shipEnt == proj.owner) continue;

        auto &sInertial = shipView.get<InertialBody>(shipEnt);
        b2Vec2 sPos = b2Body_GetPosition(sInertial.bodyId);
        float dx = pPos.x - sPos.x;
        float dy = pPos.y - sPos.y;
        if (dx*dx + dy*dy < 64.0f) { // 8 unit collision radius
            auto &stats = shipView.get<ShipStats>(shipEnt);
            stats.currentHull -= proj.damage;
            if (stats.currentHull <= 0) {
                NPCShipManager::recordCombatDeath(registry, shipEnt, proj.owner);
                registry.destroy(shipEnt);
            }
            // Projectile consumed on hit
            if (registry.valid(entity)) {
                b2DestroyBody(registry.get<InertialBody>(entity).bodyId);
                registry.destroy(entity);
            }
            break;
        }
    }
  }

  span->End();
}

entt::entity WeaponSystem::fire(::entt::registry &registry, entt::entity owner,
                                b2WorldId worldId) {
  if (!registry.valid(owner)) return entt::null;
  
  auto* weapon = registry.try_get<WeaponComponent>(owner);
  auto* trans = registry.try_get<TransformComponent>(owner);
  auto* inertial = registry.try_get<InertialBody>(owner);
  if (!weapon || !trans || !inertial || !b2Body_IsValid(inertial->bodyId) || weapon->mode == WeaponMode::Safety) return entt::null;

  if (weapon->currentCooldown > 0) return entt::null;

  auto &stats = registry.get<ShipStats>(owner);
  if (stats.isDerelict || stats.controlLoss) return entt::null;

  // 1. Resolve Attributes via Tiers (Deterministic only, QR removed)
  auto tm = [](Tier t) { 
    if (t == Tier::T2) return 3.0f;
    if (t == Tier::T3) return 8.0f;
    return 1.0f; 
  };
  float qr = 1.0f; // Baseline for all weapons (REQ-WD-DETERMINISTIC)
  
  // 2. Resource Consumption
  if (weapon->tier == WeaponTier::T1_Energy) {
    // Energy Cost: (base 10 GW) scaled by caliber tier and efficiency
    float cost = (10.0f * tm(weapon->caliberTier)) * (1.1f - static_cast<float>(weapon->efficiencyTier) * 0.1f);
    if (stats.batteryLevel < cost) return entt::null;
    stats.batteryLevel -= cost;
  } else {
    auto* mag = registry.try_get<AmmoMagazine>(owner);
    if (!mag || !mag->storedAmmo.count(weapon->selectedAmmo) || mag->storedAmmo[weapon->selectedAmmo] <= 0) return entt::null;
    mag->storedAmmo[weapon->selectedAmmo]--;
    stats.ammoStock = std::max(0.0f, stats.ammoStock - 1.0f);
  }

  // Set Cooldown: (0.8 / tm(ROF_Tier))
  weapon->currentCooldown = (0.8f / tm(weapon->rofTier));

  b2Vec2 muzzlePos = b2Body_GetPosition(inertial->bodyId);
  b2Rot rot = b2Body_GetRotation(inertial->bodyId);
  b2Vec2 direction = {rot.s, -rot.c};

  if (weapon->tier == WeaponTier::T1_Energy) {
    // Energy Beam logic: Raycast
    // Max Length: 200.0 * tm(Range_Tier)
    float range = 200.0f * tm(weapon->rangeTier);
    b2Vec2 translation = {direction.x * range, direction.y * range};
    b2RayResult result = b2World_CastRayClosest(worldId, muzzlePos, translation, b2DefaultQueryFilter());

    float finalLength = range;
    entt::entity hitEnt = entt::null;
    if (result.hit) {
        finalLength = range * result.fraction;
        b2BodyId bodyId = b2Shape_GetBody(result.shapeId);
        hitEnt = (entt::entity)(uintptr_t)b2Body_GetUserData(bodyId);
    }
    
    auto beamEnt = registry.create();
    // Damage/Tick: 5.0 * tm(Size_Tier)
    float dmg = 5.0f * tm(weapon->caliberTier);
    // Width: 1.0 + (Size_Tier - 1) * 1.5
    float width = 1.0f + (static_cast<float>(weapon->caliberTier) - 1.0f) * 1.5f;

    registry.emplace<BeamComponent>(beamEnt, 
        sf::Vector2f(muzzlePos.x, muzzlePos.y),
        sf::Vector2f(direction.x, direction.y),
        finalLength, 0.2f, 0.2f, dmg, owner, width
    );
    registry.emplace<TransformComponent>(beamEnt, trans->position);

    if (registry.valid(hitEnt)) {
        auto* targetStats = registry.try_get<ShipStats>(hitEnt);
        if (targetStats) {
            targetStats->currentHull -= dmg;
            if (targetStats->currentHull <= 0) {
                NPCShipManager::recordCombatDeath(registry, hitEnt, owner);
            }
        }
    }
    return beamEnt;
  }

  // Ballistic & Missile logic
  auto projectile = registry.create();
  auto &projComp = registry.emplace<ProjectileComponent>(projectile, weapon->baseDamage, 3.0f, owner);
  
  // Warhead Logic from selectedAmmo
  projComp.isEmp = (weapon->selectedAmmo.warhead == WarheadType::EMP);
  projComp.isExplosive = (weapon->selectedAmmo.warhead == WarheadType::Explosive);
  
  float projSpeed = weapon->projectileSpeed;

  if (weapon->tier == WeaponTier::T3_Missile) {
    auto &missile = registry.emplace<MissileComponent>(projectile);
    // Acceleration: (10.0 + tm(Weapon_Size_Tier) * 15.0)
    missile.acceleration = (10.0f + tm(weapon->caliberTier) * 15.0f) * qr;
    missile.isHeatSeeking = (weapon->selectedAmmo.guidance == GuidanceType::HeatSeeking);
    missile.isRemote = (weapon->selectedAmmo.guidance == GuidanceType::Remote);
    // Turn Rate: (1.5 + tm(Ammo_Guidance_Tier) * 2.0)
    float guidanceVal = (weapon->selectedAmmo.guidance == GuidanceType::Dumb) ? 1.0f : (weapon->selectedAmmo.guidance == GuidanceType::HeatSeeking ? 2.0f : 3.0f);
    // Note: for guidance quality mapping, we use the raw tier value for the mapping logic
    // but the multiplier for the actual physics.
    Tier guidanceTier = (weapon->selectedAmmo.guidance == GuidanceType::Dumb) ? Tier::T1 : (weapon->selectedAmmo.guidance == GuidanceType::HeatSeeking ? Tier::T2 : Tier::T3);
    missile.turnRate = (1.5f + tm(guidanceTier) * 2.0f) * qr;
    missile.maxSpeed = 20.0f + (tm(guidanceTier) * 10.0f); 
    projComp.visualScale = 1.0f;
    projComp.timeToLive = (3.0f + tm(guidanceTier) * 2.0f) * qr; 
  } else {
    // Ballistic T2
    projComp.timeToLive = 2.0f * 1.0f; // Standardized
    projComp.mass = 2.0f * tm(weapon->caliberTier);
    projComp.visualScale = 0.5f + (static_cast<float>(weapon->caliberTier) - 1.0f) * 0.75f;
    projSpeed = 1500.0f * tm(weapon->rangeTier);
  }

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = muzzlePos;
  bodyDef.isBullet = true;
  bodyDef.userData = (void *)(uintptr_t)projectile;
  b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

  b2ShapeDef shapeDef = b2DefaultShapeDef();
  b2Circle circle = {{0, 0}, 0.1f * projComp.visualScale};
  b2CreateCircleShape(bodyId, &shapeDef, &circle);

  b2Vec2 shipVel = b2Body_GetLinearVelocity(inertial->bodyId);
  b2Vec2 projRelVel = {direction.x * projSpeed, direction.y * projSpeed};
  b2Body_SetLinearVelocity(bodyId, {shipVel.x + projRelVel.x, shipVel.y + projRelVel.y});

  registry.emplace<InertialBody>(projectile, bodyId);
  registry.emplace<TransformComponent>(projectile, trans->position);

  return projectile;
}

} // namespace space
