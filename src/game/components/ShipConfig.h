#pragma once
#include "game/components/GameTypes.h"
#include "game/components/HullDef.h"
#include "game/components/ShipModule.h"
#include <map>
#include <vector>

namespace space::ShipConfig {

struct DefaultOutfitData {
  std::vector<ModuleId> engines;
  std::vector<ModuleId> weapons;
  std::vector<ModuleId> shields;
  std::vector<ModuleId> cargos;
  std::vector<ModuleId> passengers;
  std::vector<ModuleId> fuels;
  std::vector<ModuleId> powers;
};

inline std::map<Tier, DefaultOutfitData> getDefaultOutfits() {
  std::map<Tier, DefaultOutfitData> out;
  // T1: Light
  out[Tier::T1] = {{0}, {3}, {6}, {9}, {}, {}, {12}};
  // T2: Medium
  out[Tier::T2] = {{1}, {4}, {7}, {10}, {}, {}, {13}};
  // T3: Heavy (using new R-3 reactor at ID 14)
  out[Tier::T3] = {{2}, {5}, {8}, {11}, {}, {}, {14}};
  return out;
}

inline std::map<uint32_t, FactionHullTable> getFactionHulls() {
  std::map<uint32_t, FactionHullTable> out;

  FactionHullTable civ;
  civ.hulls[Tier::T1] =
      makeBasicHull("L1", "Sparrow", Tier::T1, Tier::T1, 500.f, 80.f, 20.f,
                    VisualStyle::Triangle,
                    {{0, Tier::T1, {-1.f, -1.f}, VisualStyle::Square},
                     {1, Tier::T1, {1.f, -1.f}, VisualStyle::Square}},
                    {{0, Tier::T1, {0.f, 1.f}, VisualStyle::Triangle}});

  civ.hulls[Tier::T2] =
      makeBasicHull("M1", "Falcon M1", Tier::T2, Tier::T2, 1200.f, 250.f, 50.f,
                    VisualStyle::Triangle,
                    {{0, Tier::T2, {-0.625f, 1.875f}, VisualStyle::Square},
                     {1, Tier::T1, {0.625f, 1.875f}, VisualStyle::Square}},
                    {{0, Tier::T2, {-1.25f, -0.625f}, VisualStyle::Triangle},
                     {1, Tier::T2, {1.25f, -0.625f}, VisualStyle::Triangle},
                     {2, Tier::T1, {-0.625f, -1.875f}, VisualStyle::Triangle},
                     {3, Tier::T1, {0.625f, -1.875f}, VisualStyle::Triangle}});

  civ.hulls[Tier::T3] =
      makeBasicHull("H1", "Eagle", Tier::T3, Tier::T3, 3500.f, 800.f, 150.f,
                    VisualStyle::Square,
                    {{0, Tier::T3, {-10.f, 20.f}, VisualStyle::Sleek},
                     {1, Tier::T3, {10.f, 20.f}, VisualStyle::Sleek},
                     {2, Tier::T2, {-20.f, 10.f}, VisualStyle::Sleek},
                     {3, Tier::T2, {20.f, 10.f}, VisualStyle::Sleek}},
                    {{0, Tier::T3, {-15.f, -10.f}, VisualStyle::Square},
                     {1, Tier::T3, {15.f, -10.f}, VisualStyle::Square},
                     {2, Tier::T3, {-5.f, -20.f}, VisualStyle::Square},
                     {3, Tier::T3, {5.f, -20.f}, VisualStyle::Square}});

  out[0] = civ;
  return out;
}

} // namespace space::ShipConfig
