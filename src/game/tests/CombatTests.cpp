#include <catch2/catch_all.hpp>
#include "game/components/WeaponComponent.h"
#include "game/components/AmmoComponent.h"
#include "game/components/ShipStats.h"
#include "game/components/ShipModule.h"
#include <entt/entt.hpp>
#include <cmath>

using namespace space;

TEST_CASE("Combat: Weapon Attribute Formulas", "[combat]") {
    WeaponComponent weapon;
    weapon.qualityRoll = 1.0f;
    weapon.rangeTier = Tier::T1;
    weapon.caliberTier = Tier::T1;
    weapon.rofTier = Tier::T1;
    weapon.efficiencyTier = Tier::T1;

    SECTION("Energy Beam: Range & Cost Scaling") {
        // T1: 200u
        float range = (static_cast<float>(weapon.rangeTier) * 200.0f) * weapon.qualityRoll;
        REQUIRE(range == 200.0f);

        // T1 Energy Cost (Size_Tier * 10 * (1.1 - 0.1 * Eff))
        float cost = (static_cast<float>(weapon.caliberTier) * 10.0f) * (1.1f - static_cast<float>(weapon.efficiencyTier) * 0.1f) * weapon.qualityRoll;
        REQUIRE(cost == 10.0f);

        // Upgrade Range to T3
        weapon.rangeTier = Tier::T3;
        range = (static_cast<float>(weapon.rangeTier) * 200.0f) * weapon.qualityRoll;
        REQUIRE(range == 600.0f);
    }

    SECTION("Ballistic: Kinetic Damage Calculations") {
        auto tm = [](Tier t) { return 1.0f + (static_cast<int>(t) - 1) * 0.5f; };
        
        // T1 Mass = 2.0 * 1.0 = 2.0
        float mass = 2.0f * tm(weapon.caliberTier);
        REQUIRE(mass == 2.0f);

        // Damage at 1000u/s rel. velocity
        // 0.5 * Mass * (V_rel / 100)^2
        float v_rel = 1000.0f;
        float damage = 0.5f * mass * (v_rel / 100.0f) * (v_rel / 100.0f);
        REQUIRE(damage == 100.0f);
        
        // T2 Mass = 2.0 * 1.5 = 3.0
        weapon.caliberTier = Tier::T2;
        mass = 2.0f * tm(weapon.caliberTier);
        REQUIRE(mass == 3.0f);
        damage = 0.5f * mass * (v_rel / 100.0f) * (v_rel / 100.0f);
        REQUIRE(damage == 150.0f);
    }

    SECTION("Missile: Acceleration and Steering Constraints") {
        // Accel: (10.0 + Caliber_Tier * 15.0) * QR
        float accel = (10.0f + static_cast<float>(weapon.caliberTier) * 15.0f) * weapon.qualityRoll;
        REQUIRE(accel == 25.0f); // T1: 10 + 15 = 25

        // Turn Rate: (1.5 + Guidance_Tier * 2.0) * QR
        float guidanceVal = 2.0f; // Heatseeking
        float turnRate = (1.5f + guidanceVal * 2.0f) * weapon.qualityRoll;
        REQUIRE(turnRate == 5.5f);
    }
}

TEST_CASE("Modules: Standardized Performance Scaling", "[modules]") {
    ModuleDef mod;
    mod.attributes.push_back({AttributeType::Size, Tier::T1, 1.0f});
    mod.attributes.push_back({AttributeType::Thrust, Tier::T1, 1.0f});
    mod.attributes.push_back({AttributeType::Output, Tier::T1, 1.0f});

    auto getMult = [](Tier t) {
      if (t == Tier::T1) return 1.0f;
      if (t == Tier::T2) return 3.0f;
      if (t == Tier::T3) return 8.0f;
      return 1.0f;
    };

    SECTION("Engines: Size T1 Scaling") {
        float baseThrust = 8000.0f;
        float thrust = baseThrust * getMult(mod.getAttributeTier(AttributeType::Thrust));
        REQUIRE(thrust == 8000.0f);

        // Upgrade Thrust to T3
        mod.attributes[1].tier = Tier::T3;
        thrust = baseThrust * getMult(mod.getAttributeTier(AttributeType::Thrust));
        REQUIRE(thrust == 64000.0f); // Exactly 8x
    }

    SECTION("Reactors: Size T2 Scaling") {
        mod.attributes[0].tier = Tier::T2; // Size T2
        float baseOut = 300.0f; // T2 base
        float output = baseOut * getMult(mod.getAttributeTier(AttributeType::Output));
        REQUIRE(output == 300.0f);

        // Upgrade Output to T2
        mod.attributes[2].tier = Tier::T2;
        output = baseOut * getMult(mod.getAttributeTier(AttributeType::Output));
        REQUIRE(output == 900.0f); // Exactly 3x
    }
}
