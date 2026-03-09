#include "ModuleGenerator.h"
#include <algorithm>
#include <random>

namespace space {

ModuleDef ModuleGenerator::generate(
    ModuleCategory category, const std::vector<ModuleAttribute> &attributes,
    float baseVolume, float baseMass, float baseMaint, float basePower) {
  ModuleDef def;

  std::string catName = "Module";
  switch (category) {
  case ModuleCategory::Engine:
    catName = "Engine";
    break;
  case ModuleCategory::Weapon:
    catName = "Weapon";
    break;
  case ModuleCategory::Shield:
    catName = "Shield";
    break;
  case ModuleCategory::Utility:
    catName = "Utility";
    break;
  case ModuleCategory::Reactor:
    catName = "Reactor";
    break;
  case ModuleCategory::Command:
    catName = "Command";
    break;
  case ModuleCategory::Ammo:
    catName = "Ammo Rack";
    break;
  case ModuleCategory::Battery:
    catName = "Battery";
    break;
  case ModuleCategory::ReactionWheel:
    catName = "Reaction Wheel";
    break;
  }

  Tier sizeTier = Tier::T1;
  for (auto attr : attributes) {
    if (attr.type == AttributeType::Size) {
      sizeTier = attr.tier;
    }
  }

  def.name = tierName(sizeTier) + " " + catName;
  def.category = category; // Set category for ECS dispatch
  def.volumeOccupied = baseVolume;
  def.mass = baseMass;
  def.maintenanceCost = baseMaint;
  def.powerDraw = basePower;

  for (auto attr : attributes) {
    auto type = attr.type;
    auto t = attr.tier;

    def.attributes.push_back({type, t});

    // Tiered reduction: T1=100%, T2=90%, T3=80%
    float multiplier = 1.1f - (static_cast<float>(t) * 0.1f);
    multiplier = std::max(0.1f, multiplier);

    if (type == AttributeType::Mass) {
      def.mass *= multiplier;
    } else if (type == AttributeType::Volume) {
      def.volumeOccupied *= multiplier;
    } else if (type == AttributeType::Efficiency) {
      // Efficiency reduces both maintenance and power draw (if positive)
      def.maintenanceCost *= multiplier;
      if (def.powerDraw > 0) {
        def.powerDraw *= multiplier;
      } else if (def.powerDraw < 0) {
        // For reactors, efficiency INCREASES output (output is -powerDraw)
        // 1.0/multiplier = 0.9->1.11, 0.8->1.25, 0.7->1.42
        def.powerDraw /= multiplier;
      }
    }
  }

  return def;
}

ModuleDef ModuleGenerator::generateRandomModule(ModuleCategory category,
                                                Tier sizeTier) {
  WeaponType wType = WeaponType::Energy; // Used if category == Weapon

  auto rollTier = [sizeTier]() -> Tier {
    // Attribute quality is independent of size — any tier can have any quality
    int val = rand() % 100;
    if (val < 60)
      return sizeTier;
    if (val < 80)
      return (sizeTier == Tier::T1)   ? Tier::T1
             : (sizeTier == Tier::T2) ? Tier::T1
                                      : Tier::T2;
    return (sizeTier == Tier::T1)   ? Tier::T2
           : (sizeTier == Tier::T2) ? Tier::T3
                                    : Tier::T3;
  };

  // ── Universal physical attributes (all module categories) ───────────────
  // Size   : which slot tier this module targets (always fixed to sizeTier)
  // Mass   : lightweight materials tier — higher = lighter module
  // Volume : internal volume efficiency tier — higher = smaller footprint
  std::vector<ModuleAttribute> attrs = {
      {AttributeType::Size, sizeTier},
      {AttributeType::Mass, rollTier()},
      {AttributeType::Volume, rollTier()},
  };

  // ── Category-specific functional attributes ─────────────────────────────
  // ── Category-specific functional attributes ─────────────────────────────
  switch (category) {
  case ModuleCategory::Engine:
    attrs.push_back({AttributeType::Thrust, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Weapon: {
    // Determine weapon type randomly for procedural generation
    wType = static_cast<WeaponType>(rand() % 3);

    Tier rangeTier = rollTier();
    // T1 weapons cannot have T3 range (no long-range T1 turrets rule)
    if (sizeTier == Tier::T1 && rangeTier == Tier::T3)
      rangeTier = Tier::T2;

    switch (wType) {
    case WeaponType::Energy:
      attrs.push_back({AttributeType::Range, rangeTier});
      attrs.push_back({AttributeType::Accuracy, rollTier()});
      attrs.push_back({AttributeType::ROF, rollTier()});
      attrs.push_back(
          {AttributeType::Efficiency, rollTier()}); // Energy consumption
      break;
    case WeaponType::Projectile:
      attrs.push_back({AttributeType::Range, rangeTier});
      attrs.push_back({AttributeType::Accuracy, rollTier()});
      attrs.push_back({AttributeType::ROF, rollTier()});
      attrs.push_back(
          {AttributeType::Caliber, rollTier()}); // Determines ammo size
      break;
    case WeaponType::Missile:
      attrs.push_back({AttributeType::ROF, rollTier()});
      attrs.push_back(
          {AttributeType::Caliber, rollTier()}); // Determines ammo size
      attrs.push_back(
          {AttributeType::Accuracy, rollTier()}); // Turret tracking speed
      break;
    }

    // weaponType is set down below after calling generate()
    break;
  }
  case ModuleCategory::Shield:
    attrs.push_back({AttributeType::Capacity, rollTier()});
    attrs.push_back({AttributeType::Regen, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Utility:
    // Volume here is the *cargo bay* capacity tier, distinct from the
    // physical Volume attribute above which tracks footprint efficiency
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Reactor:
    attrs.push_back({AttributeType::Output, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Command:
    // Efficiency = crew proficiency (placeholder until crew mechanics defined)
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Battery:
    // Capacity = max stored charge; Efficiency = charge efficiency;
    // Output = max discharge rate (GW)
    attrs.push_back({AttributeType::Capacity, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    attrs.push_back({AttributeType::Output, rollTier()});
    break;
  case ModuleCategory::Ammo:
    // Ammo racks just hold ammo, governed purely by their universal
    // Size and Volume attributes. No extra functional attributes.
    break;
  case ModuleCategory::ReactionWheel:
    attrs.push_back({AttributeType::TurnRate, rollTier()});
    break;
  }

  // Base values scaled from size tier
  float baseVol =
      (sizeTier == Tier::T1) ? 10.f : (sizeTier == Tier::T2 ? 30.f : 80.f);
  float baseMass = baseVol * 2.f;
  float baseMaint = baseVol * 0.5f;
  float basePower = baseVol * 1.5f;

  if (category == ModuleCategory::Reactor)
    basePower = -basePower * 2.0f; // reactors generate power (negative draw)

  ModuleDef def =
      generate(category, attrs, baseVol, baseMass, baseMaint, basePower);
  if (category == ModuleCategory::Weapon) {
    def.weaponType = wType;
  }
  return def;
}

AmmoDef ModuleGenerator::generateAmmo(WeaponType weaponType, Tier caliberTier) {
  AmmoDef ammo;
  ammo.compatibleWeapon = weaponType;
  ammo.caliber = caliberTier;

  auto rollSecondary = [caliberTier]() -> Tier {
    int val = rand() % 100;
    if (val < 60)
      return caliberTier;
    if (val < 80)
      return (caliberTier == Tier::T1)   ? Tier::T1
             : (caliberTier == Tier::T2) ? Tier::T1
                                         : Tier::T2;
    return (caliberTier == Tier::T1)   ? Tier::T2
           : (caliberTier == Tier::T2) ? Tier::T3
                                       : Tier::T3;
  };

  ammo.warhead = rollSecondary();

  if (weaponType == WeaponType::Missile) {
    ammo.range = rollSecondary();
    if (caliberTier == Tier::T1 && ammo.range == Tier::T3)
      ammo.range = Tier::T2;
    ammo.guidance = rollSecondary();
  }

  std::string name = tierName(caliberTier) + " ";
  if (weaponType == WeaponType::Projectile) {
    name += "Shells";
  } else {
    name += "Missiles";
  }
  ammo.name = name;

  // Base values
  ammo.massPerRound = (caliberTier == Tier::T1)
                          ? 1.0f
                          : (caliberTier == Tier::T2 ? 3.0f : 10.0f);
  ammo.volumePerRound = ammo.massPerRound * 0.1f;
  ammo.basePrice =
      ammo.massPerRound * 15.0f * (static_cast<int>(ammo.warhead) + 1.0f);
  if (weaponType == WeaponType::Missile) {
    ammo.basePrice *= 2.0f * (static_cast<int>(ammo.guidance) + 1.0f);
  }

  return ammo;
}

} // namespace space
