#include "ModuleGenerator.h"

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

    // Quality Roll: 0.8 to 1.2 for unique variance
    float qMult = attr.qualityRoll;
    // Tiered reduction: T1=100%, T2=90%, T3=80%
    float tierMult = 1.1f - (static_cast<float>(t) * 0.1f);
    tierMult = std::max(0.1f, tierMult);

    if (type == AttributeType::Mass) {
      def.mass *= tierMult * qMult;
    } else if (type == AttributeType::Volume) {
      def.volumeOccupied *= tierMult * qMult;
    } else if (type == AttributeType::Efficiency) {
      // Efficiency reduces both maintenance and power draw (if positive)
      def.maintenanceCost *= tierMult * qMult;
      if (def.powerDraw > 0) {
        def.powerDraw *= tierMult * qMult;
      } else if (def.powerDraw < 0) {
        // For reactors, efficiency INCREASES output (output is -powerDraw)
        // 1.0/tierMult = 0.9->1.11, 0.8->1.25, 0.7->1.42
        def.powerDraw /= (tierMult * qMult);
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
    int val = rand() % 100;
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

    // Quality Roll: 0.8 to 1.2
    attr.qualityRoll = 0.8f + (rand() % 41) / 100.f;
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
    wType = static_cast<WeaponType>(rand() % 3);

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
    // Volume here is the *cargo bay* capacity tier, distinct from the
    // physical Volume attribute above which tracks footprint efficiency
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
