//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalBoxes.hh
//---------------------------------------------------------------------------//
#include "OpticalPrism.hh"

#include <G4Box.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4Trd.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
OpticalPrism::OpticalPrism() {}

//---------------------------------------------------------------------------//
/*!
 * Construct slab geometry.
 *
 * For MSC experimental comparison, see
 * https://journals.aps.org/pr/abstract/10.1103/PhysRev.84.634
 */
G4VPhysicalVolume* OpticalPrism::Construct()
{
    return this->create_prism();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors.
 */
void OpticalPrism::ConstructSDandField()
{
    this->set_sd();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Construct slab based on input definition.
 *
 * The world volume material is vacuum and is is 4 times larger in the z-axis
 * than the input slab, while keeping the same size in x and y axes.
 */
G4VPhysicalVolume* OpticalPrism::create_prism()
{
    // Materials
    auto nist = G4NistManager::Instance();
    auto world_mat = nist->FindOrBuildMaterial("G4_Galactic");

    // World
    double const world_len = 0.5 * CLHEP::m;
    G4Box* world_box = new G4Box("world_box", world_len, world_len, world_len);
    auto const world_lv = new G4LogicalVolume(world_box, world_mat, "world_lv");
    auto const world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", nullptr, false, 0, false);

    // Equilateral optical prism
    double const prism_base = 10 * CLHEP::cm;
    double const prism_side = prism_base;
    double const prism_len = prism_base;
    auto prism_solid
        = new G4Trd("prism", prism_base, 0, prism_side, prism_side, prism_len);
    auto const prism_lv
        = new G4LogicalVolume(prism_solid, this->water_material(), "prism_lv");
    new G4PVPlacement(
        nullptr, G4ThreeVector(), prism_lv, "world_pv", world_lv, false, 0, false);

    // Add skin surface to prism
    auto opt_surface = new G4OpticalSurface("prism_surface");
    opt_surface->SetType(G4SurfaceType::dielectric_dielectric);
    opt_surface->SetFinish(G4OpticalSurfaceFinish::polished);
    opt_surface->SetModel(G4OpticalSurfaceModel::glisur);

    auto prism_skin_surface = new G4LogicalSkinSurface(
        "prism_skin_surface", prism_lv, opt_surface);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up prism as a sensitive detector.
 */
void OpticalPrism::set_sd()
{
    auto prism_sd = new SensitiveDetector("prism_sm");
    G4SDManager::GetSDMpointer()->AddNewDetector(prism_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("prism_lv", prism_sd);
}

//---------------------------------------------------------------------------//
/*!
 * Return refractive index for water.
 *
 * See Geant4's examples/extended/optical/OpNovice
 * ( \c OpNoviceDetectorConstruction::Construct )
 */
G4Material* OpticalPrism::water_material()
{
    auto* hydrogen = new G4Element(
        "hydrogen", "H", /* Z = */ 1, /* A = */ 1.01 * g / mole);
    auto* oxygen = new G4Element(
        "oxygen", "O", /* Z = */ 8, /* A = */ 16.00 * g / mole);

    auto result = new G4Material(
        "water", /* density = */ 1.0 * g / cm3, /* num_elements = */ 2);
    result->AddElement(hydrogen, 2);
    result->AddElement(oxygen, 1);

    auto prop_table = new G4MaterialPropertiesTable();
    auto const rindex = OpticalPrism::water_rindex();
    prop_table->AddProperty("RINDEX", rindex.energy, rindex.value);
    result->SetMaterialPropertiesTable(prop_table);

    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Return refractive index for water.
 *
 * See Geant4's examples/extended/optical/OpNovice
 * ( \c OpNoviceDetectorConstruction::Construct )
 */
OpticalPrism::Table OpticalPrism::water_rindex()
{
    Table result;
    result.energy = OpticalPrism::water_energy_table();
    result.value = {1.3435, 1.344,  1.3445, 1.345,  1.3455, 1.346, 1.3465,
                    1.347,  1.3475, 1.348,  1.3485, 1.3492, 1.35,  1.3505,
                    1.351,  1.3518, 1.3522, 1.3530, 1.3535, 1.354, 1.3545,
                    1.355,  1.3555, 1.356,  1.3568, 1.3572, 1.358, 1.3585,
                    1.359,  1.3595, 1.36,   1.3608};
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Return energy bins used for water properties.
 */
std::vector<double> OpticalPrism::water_energy_table()
{
    return {2.034 * eV, 2.068 * eV, 2.103 * eV, 2.139 * eV, 2.177 * eV,
            2.216 * eV, 2.256 * eV, 2.298 * eV, 2.341 * eV, 2.386 * eV,
            2.433 * eV, 2.481 * eV, 2.532 * eV, 2.585 * eV, 2.640 * eV,
            2.697 * eV, 2.757 * eV, 2.820 * eV, 2.885 * eV, 2.954 * eV,
            3.026 * eV, 3.102 * eV, 3.181 * eV, 3.265 * eV, 3.353 * eV,
            3.446 * eV, 3.545 * eV, 3.649 * eV, 3.760 * eV, 3.877 * eV,
            4.002 * eV, 4.136 * eV};
}
