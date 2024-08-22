//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalDetector.cc
//---------------------------------------------------------------------------//
#include "OpticalDetector.hh"

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4Tubs.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
OpticalDetector::OpticalDetector() {}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* OpticalDetector::Construct()
{
    return this->create_geometry();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors and (TODO) magnetic field.
 */
void OpticalDetector::ConstructSDandField()
{
    this->set_sd();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Set of cubes with different optical
 * properties.
 */
G4VPhysicalVolume* OpticalDetector::create_geometry()
{
    //// World ////

    G4Material* world_material
        = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
    world_material->SetName("vacuum");

    double const world_size = 10 * m;
    auto* world_box
        = new G4Box("world_box", world_size, world_size, world_size);
    auto const world_lv
        = new G4LogicalVolume(world_box, world_material, "world");
    auto const world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", nullptr, false, 0, false);

    //// Scintillator ////

    // Setup scint properties
    double const scint_size = 2 * m;
    auto* scint_box
        = new G4Box("scint_box", scint_size, world_size, world_size);
    auto const scint_lv
        = new G4LogicalVolume(scint_box, this->scint_material(), "world");
    G4ThreeVector scint_pos(-3 * m, 0, 0);
    auto const scint_pv = new G4PVPlacement(
        nullptr, scint_pos, scint_lv, "scint_pv", nullptr, false, 0, false);

    //// Cerenkov volume ////

#if 0
    // Setup cerenkov properties
    double const cerenkov_size = 2 * m;
    auto* cerenkov_box
        = new G4Box("cerenkov_box", cerenkov_size, world_size, world_size);
    auto const cerenkov_lv
        = new G4LogicalVolume(cerenkov_box, cerenkov_mat, "world");
    G4ThreeVector cerenkov_pos(1 * m, 0, 0);
    auto const cerenkov_pv = new G4PVPlacement(nullptr,
                                               cerenkov_pos,
                                               cerenkov_lv,
                                               "cerenkov_pv",
                                               nullptr,
                                               false,
                                               0,
                                               false);
#endif

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set up world box as a sensitive detector.
 */
void OpticalDetector::set_sd()
{
    auto world_sd = new SensitiveDetector("world_sd");
    G4SDManager::GetSDMpointer()->AddNewDetector(world_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("world", world_sd);
}

//---------------------------------------------------------------------------//
/*!
 * Return scintillation material EJ-204/NE-104/BC-404.
 *
 * This is an organic scintillator with high scintillation efficiency and
 * wavelenght around 400 nm.
 *
 * Data from https://github.com/mkandemirr/SSLG4 . This is named as OPSC-101 in
 * the README table. The spectrum can be visualized at
 * https://neutrino.erciyes.edu.tr/SSLG4/ .
 */
G4Material* OpticalDetector::scint_material()
{
    auto nist = G4NistManager::Instance();
    assert(nist);

    double const density = 1.023 * g / cm3;
    auto result = new G4Material("pvt-ej-204", density, 2);
    result->AddElement(
        nist->FindOrBuildElement("H"),
        this->to_mass_fraction("H", 5.15e+22 * (1. / cm3), density));
    result->AddElement(
        nist->FindOrBuildElement("C"),
        this->to_mass_fraction("C", 4.68e+22 * (1. / cm3), density));

    auto const scint_data = this->scint_data();
    auto const rindex_data = this->rindex_data();

    auto prop_table = new G4MaterialPropertiesTable();
    prop_table->AddProperty(
        "SCINTILLATIONCOMPONENT1", scint_data.energy, scint_data.value);
    prop_table->AddProperty("RINDEX", rindex_data.energy, rindex_data.value);
    prop_table->AddConstProperty("SCINTILLATIONYIELD", 10400 / CLHEP::MeV);
    prop_table->AddConstProperty("SCINTILLATIONYIELD1", 1 /* [unitless] */);
    prop_table->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 1.8 * CLHEP::ns);
    prop_table->AddConstProperty("SCINTILLATIONRISETIME1", 0.7 * CLHEP::ns);
    prop_table->AddConstProperty("RESOLUTIONSCALE", 1 /* [unitless] */);
    result->SetMaterialPropertiesTable(prop_table);

    return result;
}

//---------------------------------------------------------------------------//
double OpticalDetector::to_mass_fraction(std::string element_name,
                                         double atom_density,
                                         double material_density)
{
    assert(!element_name.empty());
    assert(atom_density > 0);
    assert(material_density > 0);

    auto* nist = G4NistManager::Instance();
    assert(nist);
    auto* g4_elem = nist->FindOrBuildElement(element_name);
    assert(g4_elem);

    double molar_mass = g4_elem->GetAtomicMassAmu() * (g / mole);
    double mass_density = (atom_density / CLHEP::Avogadro) * molar_mass;
    return mass_density / material_density;
}

//---------------------------------------------------------------------------//
/*!
 * Return scintillation data for EJ-204/NE-104/BC-404.
 *
 * Data from https://github.com/mkandemirr/SSLG4 . This is named as OPSC-101 in
 * the README table. The spectrum can be visualized at
 * https://neutrino.erciyes.edu.tr/SSLG4/ .
 */
OpticalDetector::Table OpticalDetector::scint_data()
{
    size_t const num_entries = 75;
    std::array<double, num_entries> wavelength = {
        380.000, 381.600, 383.200, 384.800, 386.400, 388.000, 389.600, 391.200,
        392.800, 394.400, 396.000, 397.600, 399.200, 400.800, 402.400, 404.000,
        405.600, 407.200, 408.800, 410.400, 412.000, 413.600, 415.200, 416.800,
        418.400, 420.000, 421.600, 423.200, 424.800, 426.400, 428.000, 429.600,
        431.200, 432.800, 434.400, 436.000, 437.600, 439.200, 440.800, 442.400,
        444.000, 445.600, 447.200, 448.800, 450.400, 452.000, 453.600, 455.200,
        456.800, 458.400, 460.000, 463.200, 464.800, 466.400, 468.000, 469.600,
        471.200, 472.800, 474.400, 476.000, 477.600, 479.200, 480.800, 482.400,
        484.000, 485.600, 487.200, 488.800, 490.400, 492.000, 493.600, 495.200,
        496.800, 498.400, 500.000};

    std::array<double, num_entries> amplitude = {
        0.041, 0.058, 0.085, 0.124, 0.176, 0.239, 0.316, 0.415, 0.519, 0.623,
        0.709, 0.780, 0.843, 0.884, 0.925, 0.958, 0.980, 0.997, 1.000, 0.989,
        0.961, 0.914, 0.832, 0.750, 0.678, 0.626, 0.590, 0.560, 0.538, 0.516,
        0.500, 0.489, 0.475, 0.461, 0.445, 0.431, 0.418, 0.401, 0.382, 0.365,
        0.349, 0.332, 0.310, 0.291, 0.269, 0.247, 0.223, 0.201, 0.181, 0.168,
        0.151, 0.127, 0.116, 0.107, 0.096, 0.088, 0.083, 0.074, 0.069, 0.066,
        0.061, 0.058, 0.055, 0.052, 0.047, 0.044, 0.041, 0.039, 0.036, 0.033,
        0.030, 0.025, 0.025, 0.022, 0.020};

    Table result;
    result.energy.resize(num_entries);
    result.value.resize(num_entries);
    for (auto i = 0; i < num_entries; i++)
    {
        result.energy[i] = to_energy(wavelength[i]);
        result.value[i] = amplitude[i];
    }
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Return refractive index for EJ-204/NE-104/BC-404.
 *
 * Data from https://github.com/mkandemirr/SSLG4 . This is named as OPSC-101 in
 * the README table. The spectrum can be visualized at
 * https://neutrino.erciyes.edu.tr/SSLG4/ .
 */
OpticalDetector::Table OpticalDetector::rindex_data()
{
    Table result;
    result.energy.push_back(to_energy(200));
    result.energy.push_back(to_energy(800));
    result.value.push_back(1.58);
    result.value.push_back(1.58);
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Convert wavelength [nm] to energy [MeV].
 */
double OpticalDetector::to_energy(double wavelength_nm)
{
    assert(wavelength_nm >= 0);
    return CLHEP::h_Planck * CLHEP::c_light / wavelength_nm;
}
