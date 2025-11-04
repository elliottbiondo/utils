//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OpticalPrism.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <vector>
#include <G4Material.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class OpticalPrism : public G4VUserDetectorConstruction
{
  public:
    // Construct
    OpticalPrism();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;
    // Set up sensitive detectors
    void ConstructSDandField() final;

  private:
    // Optical triangular prism
    G4VPhysicalVolume* create_prism();
    // Set volume as sensitive detector
    void set_sd();

    // Optical property data storage
    struct Table
    {
        std::vector<double> energy;  //!< [MeV]
        std::vector<double> value;  //!< General data storage
    };

    // Water with optical properties
    static G4Material* water_material();

    // Air with optical properties
    static G4Material* air_material();

    // Water refractive index (from Geant4 examples/extended/optical/OpNovice)
    static Table water_rindex();

    // Air refractive index
    static Table air_rindex();

    // Energy table data
    static std::vector<double> energy_table();
};
