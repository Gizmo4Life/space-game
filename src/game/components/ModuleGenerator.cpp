#include "ModuleGenerator.h"
#include "game/utils/RandomUtils.h"

namespace space {

ModuleDef
ModuleGenerator::generate(ModuleCategory category,
                          const std::vector<ModuleAttribute> &attributes,
                          float baseVolume, float baseMass, float baseMaint,
                          float basePower, WeaponType weaponType) {
  ModuleDef def;
  def.weaponType = weaponType;

  std::string catName = "Module";
  switch (category) {
  case ModuleCategory::Engine:
    catName = "Engine";
    break;
  case ModuleCategory::Weapon:
    switch (weaponType) {
    case WeaponType::Projectile: catName = "Autocannon"; break;
    case WeaponType::Missile:    catName = "Missile Launcher"; break;
    default:                     catName = "Energy Weapon"; break;
    }
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
  case ModuleCategory::Habitation:
    catName = "Habitation Module";
    break;
  case ModuleCategory::Cargo:
    catName = "Cargo Bay";
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

    // Quality Roll variance removed (Deterministic Tiers only)
    float qMult = 1.0f;
    
    // Functional Scaling (REQ-09): T1=1.0, T2=3.0, T3=8.0
    float functionalMult = 1.0f;
    if (t == Tier::T2) functionalMult = 3.0f;
    else if (t == Tier::T3) functionalMult = 8.0f;

    // Special case for cargo linear scaling (REQ-12)
    if (category == ModuleCategory::Cargo) {
        if (t == Tier::T2) functionalMult = 2.5f;
        else if (t == Tier::T3) functionalMult = 4.0f;
    }

    // Physical Reduction (REQ-13): T1=1.0, T2=0.9, T3=0.75
    float physicalMult = 1.0f;
    if (t == Tier::T2) physicalMult = 0.9f;
    else if (t == Tier::T3) physicalMult = 0.75f;

    if (type == AttributeType::Mass) {
      def.mass *= physicalMult * qMult;
    } else if (type == AttributeType::Volume) {
      def.volumeOccupied *= physicalMult * qMult;
    } else if (type == AttributeType::Output) {
      // Functional output (Reactors, Batteries)
      // For reactors, powerDraw is negative, so multiplying by functionalMult increases generation
      def.powerDraw *= functionalMult * qMult;
    } else if (type == AttributeType::Efficiency) {
      // Efficiency reduces both maintenance and internal power draw (if positive)
      def.maintenanceCost *= physicalMult * qMult;
      if (def.powerDraw > 0) {
        def.powerDraw *= physicalMult * qMult;
      } else if (def.powerDraw < 0) {
        // For reactors, efficiency INCREASES output (output is -powerDraw)
        // Note: Output is already scaled by functionalMult if it has an Output attribute,
        // but Efficiency provides an additional boost.
        // 1.0/tierMult = 0.9->1.11, 0.8->1.25, 0.7->1.42 (using physicalMult here for consistency)
        def.powerDraw /= (physicalMult * qMult);
      }
    }
  }

  // Calculate Base Price (REQ-11)
  float price = def.volumeOccupied * 150.f; // Base Material Cost
  float catMult = 1.0f;
  switch (category) {
  case ModuleCategory::Weapon:
    catMult = 3.0f;
    break;
  case ModuleCategory::Reactor:
    catMult = 2.5f;
    break;
  case ModuleCategory::Engine:
    catMult = 2.0f;
    break;
  case ModuleCategory::Shield:
    catMult = 1.8f;
    break;
  case ModuleCategory::Habitation:
    catMult = 1.6f;
    break;
  case ModuleCategory::Cargo:
    catMult = 1.2f;
    break;
  case ModuleCategory::Battery:
  case ModuleCategory::ReactionWheel:
    catMult = 1.4f;
    break;
  default:
    catMult = 1.0f;
  }
  price *= catMult;

  // Complexity Multiplier from Tiers
  for (const auto &attr : attributes) {
    if (attr.type == AttributeType::Size)
      continue;
    if (attr.tier == Tier::T2)
      price *= 1.5f;
    else if (attr.tier == Tier::T3)
      price *= 3.0f;
  }
  def.basePrice = price;

  return def;
}

ModuleDef ModuleGenerator::generateRandomModule(ModuleCategory category,
                                                Tier sizeTier) {
  WeaponType wType = WeaponType::Energy; // Used if category == Weapon

  auto rollAttr = [sizeTier]() -> ModuleAttribute {
    ModuleAttribute attr;
    // Attribute quality is independent of size — any tier can have any quality
    int val = Random::getInt(0, 99);
    if (val < 60)
      attr.tier = sizeTier;
    else if (val < 80)
      attr.tier = (sizeTier == Tier::T1)   ? Tier::T1
                  : (sizeTier == Tier::T2) ? Tier::T1
                                           : Tier::T2;
    else
      attr.tier = (sizeTier == Tier::T1)   ? Tier::T2
                  : (sizeTier == Tier::T2) ? Tier::T3
                                           : Tier::T3;

    // Quality Roll variance removed (Deterministic Tiers only)
    attr.qualityRoll = 1.0f;
    return attr;
  };

  // ── Universal physical attributes (all module categories) ───────────────
  // Size   : which slot tier this module targets (always fixed to sizeTier)
  // Mass   : lightweight materials tier — higher = lighter module
  // Volume : internal volume efficiency tier — higher = smaller footprint
  std::vector<ModuleAttribute> attrs = {
      {AttributeType::Size, sizeTier, 1.0f}, rollAttr(), rollAttr()};
  attrs[1].type = AttributeType::Mass;
  attrs[2].type = AttributeType::Volume;

  // ── Category-specific functional attributes ─────────────────────────────
  // ── Category-specific functional attributes ─────────────────────────────
  switch (category) {
  case ModuleCategory::Engine: {
    auto thrust = rollAttr();
    thrust.type = AttributeType::Thrust;
    attrs.push_back(thrust);
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Weapon: {
    // Determine weapon type randomly for procedural generation
    wType = static_cast<WeaponType>(Random::getInt(0, 2));

    auto range = rollAttr();
    range.type = AttributeType::Range;
    // T1 weapons cannot have T3 range (no long-range T1 turrets rule)
    if (sizeTier == Tier::T1 && range.tier == Tier::T3)
      range.tier = Tier::T2;

    switch (wType) {
    case WeaponType::Energy: {
      attrs.push_back(range);
      auto acc = rollAttr();
      acc.type = AttributeType::Accuracy;
      attrs.push_back(acc);
      auto rof = rollAttr();
      rof.type = AttributeType::ROF;
      attrs.push_back(rof);
      auto eff = rollAttr();
      eff.type = AttributeType::Efficiency;
      attrs.push_back(eff); // Energy consumption
      break;
    }
    case WeaponType::Projectile: {
      attrs.push_back(range);
      auto acc = rollAttr();
      acc.type = AttributeType::Accuracy;
      attrs.push_back(acc);
      auto rof = rollAttr();
      rof.type = AttributeType::ROF;
      attrs.push_back(rof);
      auto cal = rollAttr();
      cal.type = AttributeType::Caliber;
      attrs.push_back(cal); // Determines ammo size
      break;
    }
    case WeaponType::Missile: {
      attrs.push_back(range);
      auto rof = rollAttr();
      rof.type = AttributeType::ROF;
      attrs.push_back(rof);
      auto cal = rollAttr();
      cal.type = AttributeType::Caliber;
      attrs.push_back(cal); // Determines ammo size
      auto acc = rollAttr();
      acc.type = AttributeType::Accuracy;
      attrs.push_back(acc); // Turret tracking speed
      break;
    }
    }

    // weaponType is set down below after calling generate()
    break;
  }
  case ModuleCategory::Shield: {
    auto cap = rollAttr();
    cap.type = AttributeType::Capacity;
    attrs.push_back(cap);
    auto reg = rollAttr();
    reg.type = AttributeType::Regen;
    attrs.push_back(reg);
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Utility: {
    // Utility is now a generic catch-all, keeping it simple
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Habitation: {
    auto cap = rollAttr();
    cap.type = AttributeType::Capacity;
    attrs.push_back(cap);
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Cargo: {
    auto vol = rollAttr();
    vol.type = AttributeType::Volume;
    attrs.push_back(vol);
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Reactor: {
    auto out = rollAttr();
    out.type = AttributeType::Output;
    attrs.push_back(out);
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Command: {
    // Efficiency = crew proficiency (placeholder until crew mechanics defined)
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    break;
  }
  case ModuleCategory::Battery: {
    // Capacity = max stored charge; Efficiency = charge efficiency;
    // Output = max discharge rate (GW)
    auto cap = rollAttr();
    cap.type = AttributeType::Capacity;
    attrs.push_back(cap);
    auto eff = rollAttr();
    eff.type = AttributeType::Efficiency;
    attrs.push_back(eff);
    auto out = rollAttr();
    out.type = AttributeType::Output;
    attrs.push_back(out);
    break;
  }
  case ModuleCategory::Ammo:
    // Ammo racks just hold ammo, governed purely by their universal
    // Size and Volume attributes. No extra functional attributes.
    break;
  case ModuleCategory::ReactionWheel: {
    auto tr = rollAttr();
    tr.type = AttributeType::TurnRate;
    attrs.push_back(tr);
    break;
  }
  }

  // Base values scaled from size tier (Standard: 10/30/80)
  float baseVol =
      (sizeTier == Tier::T1) ? 10.f : (sizeTier == Tier::T2 ? 30.f : 80.f);
  float baseMass = baseVol * 2.f;  // Standard: 20t/60t/160t
  float baseMaint = baseVol * 0.5f;

  // Differentiated Power Draw (REQ-14)
  // Active systems (Engine, Shield, Weapon) draw 1.5x volume
  // Passive systems (Cargo, Habitation, Battery, etc) draw 0.2x volume
  float basePower = baseVol * 0.2f;
  if (category == ModuleCategory::Engine ||
      category == ModuleCategory::Shield ||
      category == ModuleCategory::Weapon) {
    basePower = baseVol * 1.5f; // Standard: 15/45/120 GW
  }

  if (category == ModuleCategory::Reactor)
    basePower = -(baseVol * 10.0f); // Standard: 100/300/800 GW

  ModuleDef def =
      generate(category, attrs, baseVol, baseMass, baseMaint, basePower, wType);
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
    int val = Random::getInt(0, 99);
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

  // Secondary attributes follow documented standards for base types
  ammo.warhead = rollSecondary();
  if (weaponType == WeaponType::Missile) {
    ammo.range = rollSecondary();
    ammo.guidance = rollSecondary();
  }

  auto getWarheadName = [](Tier t) -> std::string {
    if (t == Tier::T1) return "Kinetic";
    if (t == Tier::T2) return "Explosive";
    return "EMP";
  };
  auto getGuidanceName = [](Tier t) -> std::string {
    if (t == Tier::T1) return "Dumbfire";
    if (t == Tier::T2) return "Heat-Seeking";
    return "Fly-by-wire";
  };

  std::string name = tierName(caliberTier) + " ";
  if (weaponType == WeaponType::Projectile) {
    name += getWarheadName(ammo.warhead) + " Shells";
  } else {
    name += getWarheadName(ammo.warhead) + " " + getGuidanceName(ammo.guidance) + " Missiles";
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
