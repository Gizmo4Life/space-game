---
id: external-modules-guide
type: guide
pillar: external
---
[Home](/) > [Docs](/docs/readme.md) > [External](/docs/external/readme.md) > Ship Modules

# Ship Modules: A Pilot's Guide

Modules are the building blocks of your spacecraft. From the reactors that power your life support to the engines that propel you through the void, understanding module performance is key to survival.

## The Tier System

Modules are produced in three distinct tiers, representing the technological progression of the galaxy.

### T1: Basic (Civilian Grade)
- **Availability**: Found at almost any orbital station.
- **Performance**: Reliable but heavy. High mass-to-output ratio.
- **Cost**: Very affordable; uses common materials.

### T2: Industrial (Military Grade)
- **Availability**: High-tech systems and military outposts.
- **Performance**: 3x output of T1 with optimized mass.
- **Cost**: Significant investment; requires refined isotopes and alloys.

### T3: Advanced (Prototype Grade)
- **Availability**: Faction capitals and secret research labs.
- **Performance**: 8x output of T1; peak efficiency and minimal weight.
- **Cost**: Extremely expensive; requires exotic matter and blueprints.

## Universal Attributes

Every module affects your ship's physical performance in four ways:

1.  **Size**: Modules must match the slot size on your hull (T1, T2, or T3 slots).
2.  **Mass**: Heavier modules slow down your acceleration and turn rate.
3.  **Volume**: How much internal "trunk space" the module occupies.
4.  **Power Draw**: The constant energy (GW) required to keep the module active.
5.  **Capacity**: The usable containment volume provided by storage modules like Cargo Holds or Ammo Racks.

## Core Mobility Variants

While Engines provide linear thrust (`AttributeType::Thrust`) and Reactors provide power (`AttributeType::Output`), specialized modules are required for maneuverability:
- **Reaction Wheels**: Provide rotational torque (`AttributeType::Output`) to determine the ship's turn rate. Higher tier reaction wheels are required to counteract the sluggish handling caused by high-mass hulls or heavily loaded cargo bays.

## Core Storage Variants

While mobility dictates your physical movement, storage modules are critical for logistical viability:
- **Cargo Holds**: Store tradable commodities and raw resources. Standard holds scale from 50 (T1) up to 800 (T3) volume.
- **Ammo Racks**: Store physical ammunition variants for ballistic and missile weapons. Strict capacity limits are enforced (e.g., a T1 rack holds 100 units of physical volume, while a T3 rack holds 800 units).

## Deterministic Performance

Module performance is strictly determined by its attribute tiers. A "T1 Thrust" engine from one manufacturer will perform identically to a "T1 Thrust" engine from another. Functional variety arises from the combination of different attribute tiers (e.g., T3 Thrust paired with T1 Efficiency).

---

*For detailed technical specifications, see the [Module Performance Hub](../governance/standard/module-performance-standard.md).*
