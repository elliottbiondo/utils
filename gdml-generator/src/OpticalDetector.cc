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

    double const scint_size = 2 * m;
    auto* scint_box
        = new G4Box("scint_box", scint_size, world_size, world_size);
    auto const scint_lv
        = new G4LogicalVolume(scint_box, this->scint_material(), "scint_lv");
    G4ThreeVector scint_pos(-3 * m, 0, 0);
    auto const scint_pv = new G4PVPlacement(
        nullptr, scint_pos, scint_lv, "scint_pv", world_lv, false, 0, false);

    //// Water volume ////

    double const water_size = 2 * m;
    auto* water_box
        = new G4Box("water_box", water_size, world_size, world_size);
    auto const water_lv
        = new G4LogicalVolume(water_box, this->water_material(), "water_lv");
    G4ThreeVector water_box_pos(3 * m, 0, 0);
    auto const water_pv = new G4PVPlacement(
        nullptr, water_box_pos, water_lv, "water_pv", world_lv, false, 0, false);

    //// Add Rayleigh data to water ////

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Flag sensitive detectors accordingly.
 */
void OpticalDetector::set_sd()
{
    auto scint_sd = new SensitiveDetector("scint_sd");
    G4SDManager::GetSDMpointer()->AddNewDetector(scint_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("scint_lv", scint_sd);
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

    double const density = 1.023 * CLHEP::g / CLHEP::cm3;
    auto result = new G4Material("pvt-ej-204", density, 2);
    result->AddElement(
        nist->FindOrBuildElement("H"),
        this->to_mass_fraction("H", 5.15e+22 * (1. / CLHEP::cm3), density));
    result->AddElement(
        nist->FindOrBuildElement("C"),
        this->to_mass_fraction("C", 4.68e+22 * (1. / CLHEP::cm3), density));

    auto const scint_comp = this->scint_comp();
    auto const scint_rindex = this->scint_rindex();

    auto prop_table = new G4MaterialPropertiesTable();
    prop_table->AddProperty(
        "SCINTILLATIONCOMPONENT1", scint_comp.energy, scint_comp.value);
    prop_table->AddProperty("RINDEX", scint_rindex.energy, scint_rindex.value);
    prop_table->AddConstProperty("SCINTILLATIONYIELD", 10400 / CLHEP::MeV);
    prop_table->AddConstProperty("SCINTILLATIONYIELD1", 1 /* [unitless] */);
    prop_table->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 1.8 * CLHEP::ns);
    prop_table->AddConstProperty("SCINTILLATIONRISETIME1", 0.7 * CLHEP::ns);
    prop_table->AddConstProperty("RESOLUTIONSCALE", 1 /* [unitless] */);
    result->SetMaterialPropertiesTable(prop_table);

    return result;
}

//---------------------------------------------------------------------------//
/*
 * Return mass fraction of a given element given the atom density of the
 * element in the material and the material density.
 */
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
OpticalDetector::Table OpticalDetector::scint_comp()
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
        result.energy[i] = this->to_energy(wavelength[i]);
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
OpticalDetector::Table OpticalDetector::scint_rindex()
{
    Table result;
    result.energy = {this->to_energy(200), this->to_energy(800)};
    result.value = {1.58, 1.58};
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Return refractive index for water.
 *
 * See Geant4's examples/extended/optical/OpNovice
 * ( \c OpNoviceDetectorConstruction::Construct )
 */
G4Material* OpticalDetector::water_material()
{
    auto* hydrogen = new G4Element(
        "hydrogen", "H", /* Z = */ 1, /* A = */ 1.01 * g / mole);
    auto* oxygen = new G4Element(
        "oxygen", "O", /* Z = */ 8, /* A = */ 16.00 * g / mole);

    auto result = new G4Material(
        "water", /* density = */ 1.0 * g / cm3, /* num_elements = */ 2);
    result->AddElement(hydrogen, 2);
    result->AddElement(oxygen, 1);

    auto const rindex = this->water_rindex();

    auto prop_table = new G4MaterialPropertiesTable();
    prop_table->AddProperty("RINDEX", rindex.energy, rindex.value);

    Table rayleigh_mfp;
    rayleigh_mfp.energy = {rindex.energy.front(), rindex.energy.back()};
    rayleigh_mfp.value = {100 * cm, 100 * cm};  // Fake MFP values
    prop_table->AddProperty(
        "RAYLEIGH", rayleigh_mfp.energy, rayleigh_mfp.value);

    auto const absorption = this->water_absorption();
    prop_table->AddProperty("ABSLENGTH", absorption.energy, absorption.value);

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
OpticalDetector::Table OpticalDetector::water_rindex()
{
    Table result;
    result.energy = this->water_energy_table();
    result.value = {1.3435, 1.344,  1.3445, 1.345,  1.3455, 1.346, 1.3465,
                    1.347,  1.3475, 1.348,  1.3485, 1.3492, 1.35,  1.3505,
                    1.351,  1.3518, 1.3522, 1.3530, 1.3535, 1.354, 1.3545,
                    1.355,  1.3555, 1.356,  1.3568, 1.3572, 1.358, 1.3585,
                    1.359,  1.3595, 1.36,   1.3608};
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Return absorption values for water.
 *
 * See Geant4's examples/extended/optical/OpNovice
 * ( \c OpNoviceDetectorConstruction::Construct )
 */
OpticalDetector::Table OpticalDetector::water_absorption()
{
    Table result;
    result.energy = this->water_energy_table();
    result.value = {3.448 * m,  4.082 * m,  6.329 * m,  9.174 * m,  12.346 * m,
                    13.889 * m, 15.152 * m, 17.241 * m, 18.868 * m, 20.000 * m,
                    26.316 * m, 35.714 * m, 45.455 * m, 47.619 * m, 52.632 * m,
                    52.632 * m, 55.556 * m, 52.632 * m, 52.632 * m, 47.619 * m,
                    45.455 * m, 41.667 * m, 37.037 * m, 33.333 * m, 30.000 * m,
                    28.500 * m, 27.000 * m, 24.500 * m, 22.000 * m, 19.500 * m,
                    17.500 * m, 14.500 * m};
    return result;
}

//---------------------------------------------------------------------------//
/*!
 * Return energy bins used for water properties.
 */
std::vector<double> OpticalDetector::water_energy_table()
{
    return {2.034 * eV, 2.068 * eV, 2.103 * eV, 2.139 * eV, 2.177 * eV,
            2.216 * eV, 2.256 * eV, 2.298 * eV, 2.341 * eV, 2.386 * eV,
            2.433 * eV, 2.481 * eV, 2.532 * eV, 2.585 * eV, 2.640 * eV,
            2.697 * eV, 2.757 * eV, 2.820 * eV, 2.885 * eV, 2.954 * eV,
            3.026 * eV, 3.102 * eV, 3.181 * eV, 3.265 * eV, 3.353 * eV,
            3.446 * eV, 3.545 * eV, 3.649 * eV, 3.760 * eV, 3.877 * eV,
            4.002 * eV, 4.136 * eV};
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
