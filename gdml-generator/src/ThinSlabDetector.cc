//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file ThinSlabDetector.cc
//---------------------------------------------------------------------------//
#include "ThinSlabDetector.hh"

#include <G4Box.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4Tubs.hh>
#include <G4VisAttributes.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
ThinSlabDetector::ThinSlabDetector() {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* ThinSlabDetector::Construct()
{
    return this->create_slab();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors.
 */
void ThinSlabDetector::ConstructSDandField()
{
    this->set_sd();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: thin slab of Pb of 5 cm x 5 cm x 5 um.
 */
G4VPhysicalVolume* ThinSlabDetector::create_slab()
{
    // Materials
    auto nist = G4NistManager::Instance();
    auto world_mat = nist->FindOrBuildMaterial("G4_Galactic");
    auto slab_mat = nist->FindOrBuildMaterial("G4_Pb");
    world_mat->SetName("vacuum");
    slab_mat->SetName("Pb");

    // Slab dimensions
    double const slab_side = 5 * cm;
    double const slab_width = 5 * um;

    // World
    G4Box* world_box = new G4Box("world_box", slab_side, slab_side, 1 * cm);
    auto const world_lv = new G4LogicalVolume(world_box, world_mat, "world");
    auto const world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", nullptr, false, 0, false);

    // Thin slab
    G4Box* slab_box = new G4Box("slab_box", slab_side, slab_side, slab_width);
    auto const slab_lv = new G4LogicalVolume(slab_box, slab_mat, "slab");
    new G4PVPlacement(
        nullptr, G4ThreeVector(), slab_lv, "world_pv", world_lv, false, 0, false);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up slab as a sensitive detector.
 */
void ThinSlabDetector::set_sd()
{
    auto slab_sb = new SensitiveDetector("slab_sd");
    G4SDManager::GetSDMpointer()->AddNewDetector(slab_sb);
    G4VUserDetectorConstruction::SetSensitiveDetector("slab", slab_sb);
}
