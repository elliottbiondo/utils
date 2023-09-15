//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file BoxDetector.cc
//---------------------------------------------------------------------------//
#include "BoxDetector.hh"

#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4Colour.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
BoxDetector::BoxDetector() {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* BoxDetector::Construct()
{
    return this->create_box();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors and (TODO) magnetic field.
 */
void BoxDetector::ConstructSDandField()
{
    this->set_sd();

    // TODO: Add magnetic field
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Cube made of Pb with 500 m side.
 */
G4VPhysicalVolume* BoxDetector::create_box()
{
    // World material
    const char* world_material_name = "G4_Pb";
    G4Material* world_material
        = G4NistManager::Instance()->FindOrBuildMaterial(world_material_name);

    // Remove G4 prefix from material name
    world_material->SetName("Pb");

    // Size of World volume
    const double world_size = 500 * m;

    // Solid
    G4Box* world_box
        = new G4Box("world_box", world_size, world_size, world_size);

    // Logical volume
    const auto world_lv
        = new G4LogicalVolume(world_box, world_material, "world");

    // Physical volume
    const auto world_pv = new G4PVPlacement(0,               // Rotation matrix
                                            G4ThreeVector(), // Position
                                            world_lv,        // Current LV
                                            "world_pv",      // Name
                                            nullptr,         // Mother LV
                                            false,           // Bool operation
                                            0,               // Copy number
                                            false);          // Overlap check

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up world box as a sensitive detector.
 */
void BoxDetector::set_sd()
{
    auto world_sd = new SensitiveDetector("world_sd");
    G4SDManager::GetSDMpointer()->AddNewDetector(world_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("world", world_sd);
}
