#include "ModuleGenerator.h"
#include <algorithm>
#include <random>

namespace space {

ModuleDef
ModuleGenerator::generate(const std::string &baseName, Tier baseTier,
                          const std::vector<AttributeType> &requiredAttrs,
                          float baseVolume, float baseMass, float baseMaint,
                          float basePower) {
  static std::mt19937 gen(std::random_device{}());

  ModuleDef def;
  def.name = baseName;
  def.volumeOccupied = baseVolume;
  def.mass = baseMass;
  def.maintenanceCost = baseMaint;
  def.powerDraw = basePower;

  for (auto type : requiredAttrs) {
    // Randomize tier: 70% baseTier, 15% baseTier-1, 15% baseTier+1
    int tInt = static_cast<int>(baseTier);
    std::uniform_int_distribution<> dis(0, 99);
    int roll = dis(gen);

    if (roll < 15 && tInt > 1)
      tInt--;
    else if (roll > 85 && tInt < 3)
      tInt++;

    Tier t = static_cast<Tier>(tInt);
    def.attributes.push_back({type, t});

    // Tiered reduction: T1=100%, T2=90%, T3=80%, T4=70%
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

} // namespace space
