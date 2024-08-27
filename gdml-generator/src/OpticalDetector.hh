//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalDetector.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4Material.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class OpticalDetector : public G4VUserDetectorConstruction
{
  public:
    // Construct
    OpticalDetector();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;
    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;

  private:
    // Optical property data storage
    struct Table
    {
        std::vector<double> energy;  //!< [MeV]
        std::vector<double> value;  //!< General data storage
    };

  private:
    // Optical box with scintillator and Cerenkov volumes
    G4VPhysicalVolume* create_geometry();

    // Set volume as sensitive detector
    void set_sd();

    // Construct scintillator material EJ-204/NE-104/BC-404
    G4Material* scint_material();

    // EJ-204/NE-104/BC-404 data (see OPSC-101 at github.com/mkandemirr/SSLG4)
    Table scint_comp();

    // EJ-204/NE-104/BC-404 data (see OPSC-101 at github.com/mkandemirr/SSLG4)
    Table scint_rindex();

    // Construct Cerenkov material (water)
    G4Material* water_material();

    // Water refractive index (from Geant4 examples/extended/optical/OpNovice)
    Table water_rindex();

    // Water absorption values (from Geant4 examples/extended/optical/OpNovice)
    Table water_absorption();

    // Water energy table data
    std::vector<double> water_energy_table();

    // Return mass fraction of a given element
    double to_mass_fraction(std::string element_name,
                            double atomic_density,
                            double material_density);

    // Convert wavelength [nm] to energy [MeV]
    double to_energy(double wavelength_nm);
};
