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
  case ModuleCategory::Battery:
    catName = "Battery";
    break;
  }

  Tier sizeTier = Tier::T1;
  for (auto attr : attributes) {
    if (attr.type == AttributeType::Size) {
      sizeTier = attr.tier;
    }
  }

  def.name = tierName(sizeTier) + " " + catName;
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
  std::vector<ModuleAttribute> attrs;
  attrs.push_back({AttributeType::Size, sizeTier});

  auto rollTier = [sizeTier]() -> Tier {
    // 60% chance for same tier, 20% for -1, 20% for +1
    int val = rand() % 100;
    if (val < 60)
      return sizeTier;
    if (val < 80)
      return (sizeTier == Tier::T1)
                 ? Tier::T1
                 : (sizeTier == Tier::T2 ? Tier::T1 : Tier::T2);
    return (sizeTier == Tier::T1)
               ? Tier::T2
               : (sizeTier == Tier::T2 ? Tier::T3 : Tier::T3);
  };

  switch (category) {
  case ModuleCategory::Engine:
    attrs.push_back({AttributeType::Thrust, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    attrs.push_back({AttributeType::Mass, rollTier()});
    break;
  case ModuleCategory::Weapon: {
    Tier rangeTier = rollTier();
    // Restrictions for T1 weapons having excessive range
    if (sizeTier == Tier::T1 && rangeTier == Tier::T3) {
      rangeTier = Tier::T2;
    }
    attrs.push_back({AttributeType::Warhead, rollTier()});
    attrs.push_back({AttributeType::ROF, rollTier()});
    attrs.push_back({AttributeType::Range, rangeTier});
    attrs.push_back({AttributeType::Accuracy, rollTier()});
    break;
  }
  case ModuleCategory::Shield:
    attrs.push_back({AttributeType::Capacity, rollTier()});
    attrs.push_back({AttributeType::Regen, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Utility:
    attrs.push_back({AttributeType::Volume, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Reactor:
    attrs.push_back({AttributeType::Output, rollTier()});
    attrs.push_back({AttributeType::Efficiency, rollTier()});
    break;
  case ModuleCategory::Command:
    attrs.push_back({AttributeType::Command, rollTier()});
    break;
  case ModuleCategory::Battery:
    attrs.push_back({AttributeType::Battery, rollTier()});
    break;
  }

  // Base values computed from size tier
  float baseVol =
      (sizeTier == Tier::T1) ? 10.f : (sizeTier == Tier::T2 ? 30.f : 80.f);
  float baseMass = baseVol * 2.f;
  float baseMaint = baseVol * 0.5f;
  float basePower = baseVol * 1.5f;

  if (category == ModuleCategory::Reactor) {
    basePower = -basePower * 2.0f; // Reacters generate power (negative draw)
  }

  return generate(category, attrs, baseVol, baseMass, baseMaint, basePower);
}

} // namespace space
