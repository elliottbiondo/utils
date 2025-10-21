//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file MucfTestGeo.cc
//---------------------------------------------------------------------------//
#include "MucfTestGeo.hh"

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
MucfTestGeo::MucfTestGeo() {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* MucfTestGeo::Construct()
{
    return this->build_geometry();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors.
 */
void MucfTestGeo::ConstructSDandField()
{
    this->set_sd();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * MuCF test geometry.
 *
 * Comprises a d-t mixture target and neutron counters.
 * Based on https://github.com/nklabs/muonic-atom-processes .
 */
G4VPhysicalVolume* MucfTestGeo::build_geometry()
{
    using NumIsotopeComponents = int;
    using IsotopeAtomicNumber = int;
    using NumberOfNucleons = int;
    using MoleMass = double;

    G4NistManager* nist = G4NistManager::Instance();

    double const g_per_mole = CLHEP::g / CLHEP::mole;

    // Isotopes and elemental definition
    auto* hydrogen_iso = new G4Isotope("P",
                                       NumIsotopeComponents{1},
                                       NumberOfNucleons{1},
                                       MoleMass{1.00794 * g_per_mole});
    auto* hydrogen_elem
        = new G4Element("Protium", "P", NumIsotopeComponents{1});
    hydrogen_elem->AddIsotope(hydrogen_iso, /* abundance fraction */ 1);

    auto* deuteron_iso = new G4Isotope("D",
                                       IsotopeAtomicNumber{1},
                                       NumberOfNucleons{2},
                                       MoleMass{2.01410178 * g_per_mole});
    auto* deuteron_elem
        = new G4Element("Deuterium", "D", NumIsotopeComponents{1});
    deuteron_elem->AddIsotope(deuteron_iso, /* abundance fraction */ 1);

    auto* triton_iso = new G4Isotope("T",
                                     IsotopeAtomicNumber{1},
                                     NumberOfNucleons{3},
                                     MoleMass{3.0160492 * g_per_mole});
    auto* triton_elem = new G4Element("Tritium", "T", NumIsotopeComponents{1});
    triton_elem->AddIsotope(triton_iso, /* abundance fraction */ 1);

    // Muon-catalyzed fusion target mixture (hydrogen + deuteron + triton)
    double hydrogen_fraction = 1.0 - target_.deuteron_fraction
                               - target_.triton_fraction;

    if (hydrogen_fraction < 0 || hydrogen_fraction > 1)
    {
        std::cout << "fuelTritiumFraction and/or fuelDeuteriumFraction out of "
                     "range."
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (hydrogen_fraction < 2 * std::numeric_limits<double>::epsilon())
    {
        // Force to zero for very small fractions
        hydrogen_fraction = 0;
    }

    double hydrogen_density = hydrogen_fraction * 1.007825031898 * g_per_mole
                              * liquid_hydrogen_density_ * target_.density;
    double deuteron_density = target_.deuteron_fraction * 2.01410178
                              * g_per_mole * liquid_hydrogen_density_
                              * target_.density;
    double triton_density = target_.triton_fraction * 3.0160492 * g_per_mole
                            * liquid_hydrogen_density_ * target_.density;
    double material_density = hydrogen_density + deuteron_density
                              + triton_density;

    auto* target_material = new G4Material("HDTfuel",
                                           material_density,
                                           NumIsotopeComponents{3},
                                           kStateGas,
                                           target_.temperature);
    target_material->AddElement(hydrogen_elem,
                                hydrogen_density / material_density);
    target_material->AddElement(deuteron_elem,
                                deuteron_density / material_density);
    target_material->AddElement(triton_elem, triton_density / material_density);

    double env_size_xy = 40. * CLHEP::cm, env_size_z = 40. * CLHEP::cm;

    // World volume definition
    G4double world_size_xy = 2.2 * env_size_xy;
    G4double world_size_z = 2.2 * env_size_z;
    G4Material* world_material = nist->FindOrBuildMaterial("G4_AIR");

    auto* world_solid = new G4Box("world_solid",
                                  0.6 * world_size_xy,
                                  0.6 * world_size_xy,
                                  0.6 * world_size_z);
    auto* world_lv
        = new G4LogicalVolume(world_solid, world_material, "world_lv");
    auto* world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", 0, false, 0, true);

    // Target volume definition
    G4double center_pos_x = 35.0 * CLHEP::cm;

    auto* target_solid = new G4Box("target_solid",
                                   0.1 * env_size_xy,
                                   0.025 * env_size_xy,
                                   0.025 * env_size_z);
    auto* target_lv
        = new G4LogicalVolume(target_solid, target_material, "target_lv");
    new G4PVPlacement(nullptr,
                      G4ThreeVector(center_pos_x, 0., 0.),
                      target_lv,
                      "target_pv",
                      world_lv,
                      false,
                      0,
                      true);

    // Neutron detectors
    // todo: add the EPJ material soon. Only SD; no optical properties
    // Array of transverse dets (16 cm x 8 cm x 2 cm)
    double detector_x = 16 * CLHEP::cm;
    double detector_y = 2 * CLHEP::cm;
    double detector_z = 8 * CLHEP::cm;

    auto* scintillator_material
        = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    auto* detector_t_solid = new G4Box(
        "det_t_solid", 0.5 * detector_x, 0.5 * detector_y, 0.5 * detector_z);
    auto* detector_t_lv = new G4LogicalVolume(
        detector_t_solid, scintillator_material, "det_t_lv");

    double detector_t_yz_pos = 5.0 * CLHEP::cm;
    double rotation = 0;

    for (int copy_num = 0; copy_num < 4; copy_num++)
    {
        auto* rotation_matrix = new G4RotationMatrix();
        double rotation_angle = copy_num * 90.0 * CLHEP::deg;
        rotation_matrix->rotateX(rotation_angle);
        auto detector_pos = G4ThreeVector(
            center_pos_x,
            detector_t_yz_pos * cos(rotation + (90.0 * CLHEP::deg) * copy_num),
            detector_t_yz_pos * sin(rotation + (90.0 * CLHEP::deg) * copy_num));

        new G4PVPlacement(rotation_matrix,
                          detector_pos,
                          detector_t_lv,
                          "det_t_pv",
                          world_lv,
                          false,
                          copy_num,
                          true);
    }

    // Back detector with 2 cm center hole (8 cm x 8 cm x 2 cm)
    auto* detector_back_solid_full = new G4Box("det_b_solid_full",
                                               0.5 * detector_y,
                                               0.5 * detector_z,
                                               0.5 * detector_z);
    auto* hole = new G4Tubs("hole_solid",
                            0 * CLHEP::cm,
                            1 * CLHEP::cm,
                            detector_y,
                            0,
                            360 * CLHEP::deg);

    auto* rotation_matrix_z = new G4RotationMatrix();
    rotation_matrix_z->rotateY(90 * CLHEP::deg);

    auto* detector_back_solid = new G4SubtractionSolid("det_b_solid",
                                                       detector_back_solid_full,
                                                       hole,
                                                       rotation_matrix_z,
                                                       G4ThreeVector());

    auto* detector_back_lv = new G4LogicalVolume(
        detector_back_solid, scintillator_material, "det_b_lv");
    new G4PVPlacement(nullptr,
                      G4ThreeVector(26 * CLHEP::cm, 0, 0),
                      detector_back_lv,
                      "det_b_pv",
                      world_lv,
                      false,
                      0,
                      true);

    // Forward detector (8 cm x 8 cm x 2 cm)
    G4Box* detector_f_solid = new G4Box(
        "det_f_solid", 0.5 * detector_y, 0.5 * detector_z, 0.5 * detector_z);

    auto* detector_f_lv = new G4LogicalVolume(
        detector_f_solid, scintillator_material, "det_f_lv");
    new G4PVPlacement(nullptr,
                      G4ThreeVector(44 * CLHEP::cm, 0, 0),
                      detector_f_lv,
                      "det_f_pv",
                      world_lv,
                      false,
                      0,
                      true);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up sensitive detectors.
 */
void MucfTestGeo::set_sd()
{
    auto sd_manager = G4SDManager::GetSDMpointer();

    auto target_sd = new SensitiveDetector("target_sd");
    G4VUserDetectorConstruction::SetSensitiveDetector("target_lv", target_sd);
    sd_manager->AddNewDetector(target_sd);

    auto det_t_sd = new SensitiveDetector("det_t_sd");
    G4VUserDetectorConstruction::SetSensitiveDetector("det_t_lv", det_t_sd);
    sd_manager->AddNewDetector(det_t_sd);

    auto det_f_sd = new SensitiveDetector("det_f_sd");
    G4VUserDetectorConstruction::SetSensitiveDetector("det_f_lv", det_f_sd);
    sd_manager->AddNewDetector(det_f_sd);

    auto det_b_sd = new SensitiveDetector("det_b_sd");
    G4VUserDetectorConstruction::SetSensitiveDetector("det_b_lv", det_b_sd);
    sd_manager->AddNewDetector(det_b_sd);
}
