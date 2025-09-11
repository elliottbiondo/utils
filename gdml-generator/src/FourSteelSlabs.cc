//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file FourSteelSlabs.cc
//---------------------------------------------------------------------------//
#include "FourSteelSlabs.hh"

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
FourSteelSlabs::FourSteelSlabs() {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* FourSteelSlabs::Construct()
{
    return this->create_geometry();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors.
 */
void FourSteelSlabs::ConstructSDandField()
{
    this->set_sd();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: World volume with 4 steel slabs.
 *
 * \note G4Replicas are avoided.
 */
G4VPhysicalVolume* FourSteelSlabs::create_geometry()
{
    // Geometry materials
    auto const nist_manager = G4NistManager::Instance();
    auto const world_material
        = nist_manager->FindOrBuildMaterial("G4_Galactic");
    auto const slab_material
        = nist_manager->FindOrBuildMaterial("G4_STAINLESS-STEEL");

    // World definition
    double const world_size = 1000. * cm;
    double const half_world = world_size * 0.5;

    auto const world_solid
        = new G4Box("world_box", half_world, half_world, half_world);

    auto const world_lv
        = new G4LogicalVolume(world_solid, world_material, "world_lv");

    auto const world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", nullptr, false, 0, true);

    // Slabs definition
    auto const slabs_xy = 0.01 * world_size;
    auto const slabs_z = 0.2 * slabs_xy;
    auto const slab_solid = new G4Box("box", slabs_xy, slabs_xy, slabs_z);
    auto const slab_lv
        = new G4LogicalVolume(slab_solid, slab_material, "box_lv");

    // Placements
    new G4PVPlacement(
        nullptr, G4ThreeVector(), slab_lv, "box0_pv", world_lv, false, 0, true);

    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, 3 * slabs_z),
                      slab_lv,
                      "box1_pv",
                      world_lv,
                      false,
                      0,
                      true);

    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, 6 * slabs_z),
                      slab_lv,
                      "box2_pv",
                      world_lv,
                      false,
                      0,
                      true);

    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, 9 * slabs_z),
                      slab_lv,
                      "box3_pv",
                      world_lv,
                      false,
                      0,
                      true);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set the 4 slab as a sensitive detectors.
 */
void FourSteelSlabs::set_sd()
{
    auto sd = new SensitiveDetector("box_sd");
    G4VUserDetectorConstruction::SetSensitiveDetector("box_lv", sd);
    G4SDManager::GetSDMpointer()->AddNewDetector(sd);
}
