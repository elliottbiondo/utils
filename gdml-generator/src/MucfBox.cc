//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file MucfBox.cc
//---------------------------------------------------------------------------//
#include "MucfBox.hh"

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4RotationMatrix.hh>
#include <G4SDManager.hh>
#include <G4SubtractionSolid.hh>
#include <G4Transform3D.hh>
#include <G4Tubs.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
MucfBox::MucfBox() {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* MucfBox::Construct()
{
    return this->build_target_box();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors.
 */
void MucfBox::ConstructSDandField()
{
    this->set_sd_target_box();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * MuCF test box.
 *
 * Comprises a d-t mixture target only.
 */
G4VPhysicalVolume* MucfBox::build_target_box()
{
    using NumElementComponents = int;
    using NumIsotopeComponents = int;
    using IsotopeAtomicNumber = int;
    using NumberOfNucleons = int;
    using MoleMass = double;

    double const g_per_mole = CLHEP::g / CLHEP::mole;
    MoleMass const hydrogen_mole_mass{1.00794 * g_per_mole};
    MoleMass const deuteron_mole_mass{2.01410178 * g_per_mole};
    MoleMass const triton_mole_mass{3.0160492 * g_per_mole};
    double const temperature = 300 * CLHEP::kelvin;
    double const target_lhd_density = 1;
    double const liquid_hydrogen_density_
        = 4.25e22 / CLHEP::Avogadro * CLHEP::mole / CLHEP::cm3;  //!< atoms/cm3

    // Isotopes and elemental definition
    auto* hydrogen_iso = new G4Isotope("protium",
                                       NumIsotopeComponents{1},
                                       NumberOfNucleons{1},
                                       hydrogen_mole_mass);
    auto* deuteron_iso = new G4Isotope("deuterium",
                                       IsotopeAtomicNumber{1},
                                       NumberOfNucleons{2},
                                       deuteron_mole_mass);
    auto* triton_iso = new G4Isotope("tritium",
                                     IsotopeAtomicNumber{1},
                                     NumberOfNucleons{3},
                                     triton_mole_mass);

    auto* hydrogen_elem
        = new G4Element("hydrogen-mix", "H", NumIsotopeComponents{3});

    double const fraction_h = 0.0;
    double const fraction_d = 0.5;
    double const fraction_t = 0.5;
    hydrogen_elem->AddIsotope(hydrogen_iso, fraction_h);
    hydrogen_elem->AddIsotope(deuteron_iso, fraction_d);
    hydrogen_elem->AddIsotope(triton_iso, fraction_t);

    double const hydrogen_density = fraction_h * hydrogen_mole_mass
                                    * target_lhd_density
                                    * liquid_hydrogen_density_;
    double const deuteron_density = fraction_d * deuteron_mole_mass
                                    * target_lhd_density
                                    * liquid_hydrogen_density_;
    double const triton_density = fraction_t * triton_mole_mass
                                  * target_lhd_density
                                  * liquid_hydrogen_density_;

    double const material_density = hydrogen_density + deuteron_density
                                    + triton_density;

    auto* target_material = new G4Material("hdt_fuel",
                                           material_density,
                                           NumElementComponents{1},
                                           kStateGas,
                                           temperature);
    target_material->AddElement(hydrogen_elem, 1);

    // Create and place world volume
    double const box_side = 50 * CLHEP::cm;
    auto* world_solid = new G4Box("world_solid", box_side, box_side, box_side);
    auto* world_lv
        = new G4LogicalVolume(world_solid, target_material, "world_lv");
    auto* world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", 0, false, 0, true);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up sensitive detectors.
 */
void MucfBox::set_sd_target_box()
{
    auto sd_manager = G4SDManager::GetSDMpointer();

    auto world_sd = new SensitiveDetector("world_sd");
    G4VUserDetectorConstruction::SetSensitiveDetector("world_lv", world_sd);
    sd_manager->AddNewDetector(world_sd);
}
