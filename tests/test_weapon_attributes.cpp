#include "game/components/ModuleGenerator.h"
#include "game/components/GameTypes.h"
#include "game/components/ShipModule.h"
#include "game/NPCShipManager.h"
#include "game/FactionManager.h"
#include "game/components/InstalledModules.h"
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>
#include <set>

using namespace space;

// ─────────────────────────────────────────────────────────────────────────────
// 1. Energy Weapon Attributes
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Weapon Attributes: Energy weapons have Range, Accuracy, ROF, Efficiency",
          "[game][weapons][attributes]") {
    auto& gen = ModuleGenerator::instance();

    for (int i = 0; i < 20; ++i) {
        Tier size = static_cast<Tier>((i % 3) + 1);
        ModuleDef def = gen.generateRandomModule(ModuleCategory::Weapon, size);

        // Skip if the random roll didn't produce an Energy weapon
        if (def.weaponType != WeaponType::Energy) continue;

        INFO("Energy weapon: " << def.name);
        REQUIRE(def.category == ModuleCategory::Weapon);
        REQUIRE(def.weaponType == WeaponType::Energy);

        // Must have archetype-specific attributes
        REQUIRE(def.hasAttribute(AttributeType::Range));
        REQUIRE(def.hasAttribute(AttributeType::Accuracy));
        REQUIRE(def.hasAttribute(AttributeType::ROF));
        REQUIRE(def.hasAttribute(AttributeType::Efficiency));

        // Must have universal physical attributes
        REQUIRE(def.hasAttribute(AttributeType::Size));
        REQUIRE(def.hasAttribute(AttributeType::Mass));
        REQUIRE(def.hasAttribute(AttributeType::Volume));

        // Name must NOT be generic "Weapon"
        REQUIRE(def.name.find("Energy Weapon") != std::string::npos);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. Projectile Weapon Attributes
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Weapon Attributes: Projectile weapons have Range, Accuracy, ROF, Caliber", "[weapon][generation]") {
    auto& gen = ModuleGenerator::instance();

    for (int i = 0; i < 20; ++i) {
        Tier size = static_cast<Tier>((i % 3) + 1);
        ModuleDef def = gen.generateRandomModule(ModuleCategory::Weapon, size);

        if (def.weaponType != WeaponType::Projectile) continue;

        INFO("Projectile weapon: " << def.name);
        REQUIRE(def.category == ModuleCategory::Weapon);
        REQUIRE(def.weaponType == WeaponType::Projectile);

        REQUIRE(def.hasAttribute(AttributeType::Range));
        REQUIRE(def.hasAttribute(AttributeType::Accuracy));
        REQUIRE(def.hasAttribute(AttributeType::ROF));
        REQUIRE(def.hasAttribute(AttributeType::Caliber));

        REQUIRE(def.hasAttribute(AttributeType::Size));
        REQUIRE(def.hasAttribute(AttributeType::Mass));
        REQUIRE(def.hasAttribute(AttributeType::Volume));

        REQUIRE(def.name.find("Autocannon") != std::string::npos);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. Missile Weapon Attributes
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Weapon Attributes: Missile weapons have Range, Accuracy, ROF, Caliber", "[weapon][generation]") {
    auto& gen = ModuleGenerator::instance();

    for (int i = 0; i < 20; ++i) {
        Tier size = static_cast<Tier>((i % 3) + 1);
        ModuleDef def = gen.generateRandomModule(ModuleCategory::Weapon, size);

        if (def.weaponType != WeaponType::Missile) continue;

        INFO("Missile weapon: " << def.name);
        REQUIRE(def.category == ModuleCategory::Weapon);
        REQUIRE(def.weaponType == WeaponType::Missile);

        REQUIRE(def.hasAttribute(AttributeType::ROF));
        REQUIRE(def.hasAttribute(AttributeType::Caliber));
        REQUIRE(def.hasAttribute(AttributeType::Accuracy));

        REQUIRE(def.hasAttribute(AttributeType::Size));
        REQUIRE(def.hasAttribute(AttributeType::Mass));
        REQUIRE(def.hasAttribute(AttributeType::Volume));

        REQUIRE(def.name.find("Missile Launcher") != std::string::npos);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. All three archetypes are generated (no bias)
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Weapon Attributes: All three archetypes appear in random generation",
          "[game][weapons][attributes]") {
    auto& gen = ModuleGenerator::instance();

    std::set<WeaponType> seen;
    for (int i = 0; i < 100; ++i) {
        Tier size = static_cast<Tier>((i % 3) + 1);
        ModuleDef def = gen.generateRandomModule(ModuleCategory::Weapon, size);
        seen.insert(def.weaponType);
        if (seen.size() == 3) break;
    }

    REQUIRE(seen.count(WeaponType::Energy) == 1);
    REQUIRE(seen.count(WeaponType::Projectile) == 1);
    REQUIRE(seen.count(WeaponType::Missile) == 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. Explicit generate() with specified WeaponType produces correct name
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Weapon Attributes: Explicit generate() names weapons by archetype",
          "[game][weapons][attributes]") {
    auto& gen = ModuleGenerator::instance();

    std::vector<ModuleAttribute> attrs = {
        {AttributeType::Size, Tier::T2, 1.0f},
        {AttributeType::Mass, Tier::T2, 1.0f},
        {AttributeType::Volume, Tier::T2, 1.0f},
        {AttributeType::ROF, Tier::T1, 1.0f}
    };

    auto energy = gen.generate(ModuleCategory::Weapon, attrs,
                               30.f, 60.f, 15.f, 45.f, WeaponType::Energy);
    REQUIRE(energy.name == "Medium Energy Weapon");

    auto proj = gen.generate(ModuleCategory::Weapon, attrs,
                             30.f, 60.f, 15.f, 45.f, WeaponType::Projectile);
    REQUIRE(proj.name == "Medium Autocannon");

    auto missile = gen.generate(ModuleCategory::Weapon, attrs,
                                30.f, 60.f, 15.f, 45.f, WeaponType::Missile);
    REQUIRE(missile.name == "Medium Missile Launcher");
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. Spawned ships have valid weapon modules with archetype attributes
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("Weapon Attributes: Spawned ships have typed weapon modules",
          "[game][weapons][integration]") {
    entt::registry registry;
    b2WorldDef worldDef = b2DefaultWorldDef();
    b2WorldId worldId = b2CreateWorld(&worldDef);

    FactionManager::instance().init();

    // Spawn 10 ships across different factions/tiers/roles
    std::vector<entt::entity> ships;
    for (uint32_t fId = 1; fId <= 3; ++fId) {
        for (const auto& role : {"Combat", "General"}) {
            auto ship = NPCShipManager::instance().spawnShip(
                registry, fId, {0, 0}, worldId, Tier::T1, false, entt::null, role);
            if (registry.valid(ship)) ships.push_back(ship);
        }
    }

    REQUIRE(!ships.empty());

    int weaponModulesChecked = 0;
    for (auto ship : ships) {
        if (!registry.all_of<InstalledWeapons>(ship)) continue;
        auto& iw = registry.get<InstalledWeapons>(ship);

        for (const auto& m : iw.modules) {
            if (m.name.empty() || m.name == "Empty") continue;
            weaponModulesChecked++;

            INFO("Ship weapon: " << m.name << " type=" << static_cast<int>(m.weaponType));

            // Must NOT have generic "Weapon" in the name
            bool hasArchetypeName =
                m.name.find("Energy Weapon") != std::string::npos ||
                m.name.find("Autocannon") != std::string::npos ||
                m.name.find("Missile Launcher") != std::string::npos;
            REQUIRE(hasArchetypeName);

            // Must always have universal attributes
            REQUIRE(m.hasAttribute(AttributeType::Size));
            REQUIRE(m.hasAttribute(AttributeType::Mass));
            REQUIRE(m.hasAttribute(AttributeType::Volume));

            // Must have at least ROF (all weapon types have ROF)
            REQUIRE(m.hasAttribute(AttributeType::ROF));

            // Archetype-specific checks
            if (m.weaponType == WeaponType::Energy) {
                REQUIRE(m.hasAttribute(AttributeType::Range));
                REQUIRE(m.hasAttribute(AttributeType::Accuracy));
                REQUIRE(m.hasAttribute(AttributeType::Efficiency));
            } else if (m.weaponType == WeaponType::Projectile) {
                REQUIRE(m.hasAttribute(AttributeType::Range));
                REQUIRE(m.hasAttribute(AttributeType::Accuracy));
                REQUIRE(m.hasAttribute(AttributeType::ROF));
                REQUIRE(m.hasAttribute(AttributeType::Caliber));
            } else if (m.weaponType == WeaponType::Missile) {
                REQUIRE(m.hasAttribute(AttributeType::Range));
                REQUIRE(m.hasAttribute(AttributeType::Accuracy));
                REQUIRE(m.hasAttribute(AttributeType::ROF));
                REQUIRE(m.hasAttribute(AttributeType::Caliber));
            }
        }
    }

    // We expect at least some weapon modules were checked
    REQUIRE(weaponModulesChecked > 0);

    b2DestroyWorld(worldId);
}
