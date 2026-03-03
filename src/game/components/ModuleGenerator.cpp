#include "ModuleGenerator.h"
#include <algorithm>
#include <random>

namespace space {

ModuleDef
ModuleGenerator::generate(const std::string &baseName, Tier baseTier,
                          const std::vector<AttributeType> &requiredAttrs,
                          float baseVolume, float baseMaint) {
  static std::mt19937 gen(std::random_device{}());

  ModuleDef def;
  def.name = baseName;
  def.volumeOccupied = baseVolume;
  def.maintenanceCost = baseMaint;

  for (auto type : requiredAttrs) {
    // Randomize tier: 70% baseTier, 15% baseTier-1, 15% baseTier+1
    int tInt = static_cast<int>(baseTier);
    std::uniform_int_distribution<> dis(0, 99);
    int roll = dis(gen);

    if (roll < 15 && tInt > 1)
      tInt--;
    else if (roll > 85 && tInt < 3)
      tInt++;

    def.attributes.push_back({type, static_cast<Tier>(tInt)});
  }

  return def;
}

} // namespace space
