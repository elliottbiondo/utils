//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SimpleCms.cc
//---------------------------------------------------------------------------//
#include "SimpleCms.hh"

#include <G4Box.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4Tubs.hh>
#include <G4VisAttributes.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with geometry type enum.
 */
SimpleCms::SimpleCms(MaterialType type) : geometry_type_(type) {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* SimpleCms::Construct()
{
    return this->simple_cms();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors and (TODO) magnetic field.
 */
void SimpleCms::ConstructSDandField()
{
    this->set_sd();

    // TODO: Add magnetic field
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Define list of materials.
 */
SimpleCms::MaterialList SimpleCms::build_materials()
{
    MaterialList materials;
    G4NistManager* nist = G4NistManager::Instance();

    switch (geometry_type_)
    {
        case MaterialType::simple:
            // Load materials
            materials.world = nist->FindOrBuildMaterial("G4_Galactic");
            materials.vacuum_tube = nist->FindOrBuildMaterial("G4_Galactic");
            materials.si_tracker = nist->FindOrBuildMaterial("G4_Si");
            materials.em_calorimeter = nist->FindOrBuildMaterial("G4_Pb");
            materials.had_calorimeter = nist->FindOrBuildMaterial("G4_C");
            materials.sc_solenoid = nist->FindOrBuildMaterial("G4_Ti");
            materials.muon_chambers = nist->FindOrBuildMaterial("G4_Fe");

            // Update names
            materials.world->SetName("vacuum");
            materials.vacuum_tube->SetName("vacuum");
            materials.si_tracker->SetName("Si");
            materials.em_calorimeter->SetName("Pb");
            materials.had_calorimeter->SetName("C");
            materials.sc_solenoid->SetName("Ti");
            materials.muon_chambers->SetName("Fe");
            break;

        case MaterialType::composite:
            // Load materials
            materials.world = nist->FindOrBuildMaterial("G4_Galactic");
            materials.vacuum_tube = nist->FindOrBuildMaterial("G4_Galactic");
            materials.si_tracker
                = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
            materials.em_calorimeter
                = nist->FindOrBuildMaterial("G4_LEAD_OXIDE");
            materials.had_calorimeter = nist->FindOrBuildMaterial("G4_C");
            materials.sc_solenoid = nist->FindOrBuildMaterial("G4_Ti");
            materials.muon_chambers = nist->FindOrBuildMaterial("G4_Fe");

            // Update names
            materials.world->SetName("vacuum");
            materials.vacuum_tube->SetName("vacuum");
            materials.si_tracker->SetName("SiO2");
            materials.em_calorimeter->SetName("Pb3O4");
            materials.had_calorimeter->SetName("C");
            materials.sc_solenoid->SetName("Ti");
            materials.muon_chambers->SetName("Fe");
            break;

        default:
            break;
    }

    return materials;
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Single material CMS mock up.
 *
 * This is a set of single-material concentric cylinders that acts as a
 * cylindrical cow in a vacuum version of CMS.
 *
 * - The World volume is a box, and its dimensions are expressed in cartesian
 * coordinates [x. y, z].
 * - **All** other volumes are concentric cylinders, and their dimensions are
 * expressed as [inner radius, outer radius, length]
 *
 * | Volume                       | Composition      | Dimensions [cm]    |
 * | ---------------------------- | ---------------- | ------------------ |
 * | world                        | H                | [1000, 1000, 2000] |
 * | vacuum tube                  | H                | [0, 30, 1400]      |
 * | silicon tracker              | Si or SiO2       | [30, 125, 1400]    |
 * | electromagnetic calorimeter  | Pb or Pb3O4      | [125, 175, 1400]   |
 * | hadron calorimeter           | C                | [175, 275, 1400]   |
 * | superconducting solenoid     | Ti               | [275, 375, 1400]   |
 * | muon chambers                | Fe               | [375, 700, 1400]   |
 *
 * - Different distances between volumes are set so that geometry navigation
 * can be tested. These values are defined in \c volume_gaps_ . Current
 * configuration uses:
 *
 * | Volume                       | Gap type   |
 * | ---------------------------- | ---------- |
 * | world                        | N/A        |
 * | vacuum tube                  | tolerance  |
 * | silicon tracker              | tolerance  |
 * | electromagnetic calorimeter  | overlap    |
 * | hadron calorimeter           | overlap    |
 * | superconducting solenoid     | millimeter |
 * | muon chambers                | N/A        |
 *
 */
G4VPhysicalVolume* SimpleCms::simple_cms()
{
    // Set up material list
    MaterialList materials = this->build_materials();

    // Size of World volume
    double const world_size = 20 * m;
    // Half length of all concentric cylinders (z-axis)
    double const half_length = 7 * m;

    // List of solids
    G4Box* world_def
        = new G4Box("world_box", world_size / 2, world_size / 2, world_size);

    G4Tubs* vacuum_tube_def
        = new G4Tubs("lhc_vacuum_tube",
                     0,  // Inner radius
                     30 * cm - volume_gaps_.tolerance,  // Outer radius
                     half_length,  // Half-length z
                     0 * deg,  // Start angle
                     360 * deg);  // Spanning angle

    G4Tubs* si_tracker_def = new G4Tubs("silicon_tracker",
                                        30 * cm,
                                        125 * cm - volume_gaps_.tolerance,
                                        half_length,
                                        0 * deg,
                                        360 * deg);

    G4Tubs* em_calorimeter_def = new G4Tubs("crystal_em_calorimeter",
                                            125 * cm,
                                            175 * cm - volume_gaps_.overlap,
                                            half_length,
                                            0 * deg,
                                            360 * deg);

    G4Tubs* had_calorimeter_def = new G4Tubs("hadron_calorimeter",
                                             175 * cm,
                                             275 * cm - volume_gaps_.overlap,
                                             half_length,
                                             0 * deg,
                                             360 * deg);

    G4Tubs* sc_solenoid_def = new G4Tubs("superconducting_solenoid",
                                         275 * cm,
                                         375 * cm - volume_gaps_.millimeter,
                                         half_length,
                                         0 * deg,
                                         360 * deg);

    G4Tubs* iron_muon_chambers_def = new G4Tubs("iron_muon_chambers",
                                                375 * cm,
                                                700 * cm,
                                                half_length,
                                                0 * deg,
                                                360 * deg);

    // List of logical volumes
    auto const world_lv
        = new G4LogicalVolume(world_def, materials.world, "world");

    auto const vacuum_tube_lv = new G4LogicalVolume(
        vacuum_tube_def, materials.vacuum_tube, "vacuum_tube");

    auto const si_tracker_lv = new G4LogicalVolume(
        si_tracker_def, materials.si_tracker, "si_tracker");

    auto const em_calorimeter_lv = new G4LogicalVolume(
        em_calorimeter_def, materials.em_calorimeter, "em_calorimeter");

    auto const had_calorimeter_lv = new G4LogicalVolume(
        had_calorimeter_def, materials.had_calorimeter, "had_calorimeter");

    auto const sc_solenoid_lv = new G4LogicalVolume(
        sc_solenoid_def, materials.sc_solenoid, "sc_solenoid");

    auto const iron_muon_chambers_lv = new G4LogicalVolume(
        iron_muon_chambers_def, materials.muon_chambers, "fe_muon_chambers");

    // List of physical volumes
    auto const world_pv = new G4PVPlacement(0,  // Rotation matrix
                                            G4ThreeVector(),  // Position
                                            world_lv,  // Current LV
                                            "world_pv",  // Name
                                            nullptr,  // Mother LV
                                            false,  // Bool operation
                                            0,  // Copy number
                                            false);  // Overlap check

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      vacuum_tube_lv,
                      "vacuum_tube_pv",
                      world_lv,
                      false,
                      0,
                      false);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      si_tracker_lv,
                      "si_tracker_pv",
                      world_lv,
                      false,
                      0,
                      false);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      em_calorimeter_lv,
                      "em_calorimeter_pv",
                      world_lv,
                      false,
                      0,
                      false);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      had_calorimeter_lv,
                      "had_calorimeter_pv",
                      world_lv,
                      false,
                      0,
                      false);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      sc_solenoid_lv,
                      "sc_solenoid_pv",
                      world_lv,
                      false,
                      0,
                      false);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      iron_muon_chambers_lv,
                      "iron_muon_chambers_pv",
                      world_lv,
                      false,
                      0,
                      false);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up simple cms sensitive detectors.
 * The two first inner cylinders, which represent the silicon tracker and the
 * EM calorimeter, are used as sensitive scoring regions.
 */
void SimpleCms::set_sd()
{
    // List of sensitive detectors
    auto si_tracker_sd = new SensitiveDetector("si_tracker_sd");
    auto em_calorimeter_sd = new SensitiveDetector("em_calorimeter_sd");

    // Add SD to manager
    G4SDManager::GetSDMpointer()->AddNewDetector(si_tracker_sd);
    G4SDManager::GetSDMpointer()->AddNewDetector(em_calorimeter_sd);

    // Set appropriate logical volumes as sensitive detectors
    G4VUserDetectorConstruction::SetSensitiveDetector("si_tracker",
                                                      si_tracker_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("em_calorimeter",
                                                      em_calorimeter_sd);
}
