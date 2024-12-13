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
    return this->create_slab(this->carbon_slab_def());
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
 * Define Pb slab of 5 cm x 5 cm x 5 um.
 */
ThinSlabDetector::SlabDefinition ThinSlabDetector::lead_slab_def()
{
    auto nist = G4NistManager::Instance();
    auto slab_mat = nist->FindOrBuildMaterial("G4_Pb");
    slab_mat->SetName("Pb");

    SlabDefinition def;
    def.material = slab_mat;
    def.dimension = {5 * cm, 5 * cm, 5 * um};
    return def;
}

//---------------------------------------------------------------------------//
/*!
 * Define carbon slab of 5 cm x 5 cm x 50 um.
 */
ThinSlabDetector::SlabDefinition ThinSlabDetector::carbon_slab_def()
{
    auto nist = G4NistManager::Instance();
    auto slab_mat = nist->FindOrBuildMaterial("G4_C");
    slab_mat->SetName("C");

    SlabDefinition def;
    def.material = slab_mat;
    def.dimension = {5 * cm, 5 * cm, 50 * um};
    return def;
}

//---------------------------------------------------------------------------//
/*!
 * Construct slab based on input definition. The world volume is vacuum and is
 * is 4 times larger in the z-axis than the input slab, while keeping the same
 * size in x and y axes.
 */
G4VPhysicalVolume* ThinSlabDetector::create_slab(SlabDefinition const& def)
{
    // Materials
    auto nist = G4NistManager::Instance();
    auto world_mat = nist->FindOrBuildMaterial("G4_Galactic");
    world_mat->SetName("vacuum");

    auto const& dim = def.dimension;

    // World
    G4Box* world_box = new G4Box("world_box", dim[0], dim[1], 4 * dim[2]);
    auto const world_lv = new G4LogicalVolume(world_box, world_mat, "world");
    auto const world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", nullptr, false, 0, false);

    // Thin slab
    G4Box* slab_box = new G4Box("slab_box", dim[0], dim[1], dim[2]);
    auto const slab_lv = new G4LogicalVolume(slab_box, def.material, "slab");
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
