//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file MucfTestGeo.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <G4SystemOfUnits.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class MucfTestGeo : public G4VUserDetectorConstruction
{
  public:
    // Construct
    MucfTestGeo();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;

    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;

  private:
    // MuCF test geometry
    G4VPhysicalVolume* build_geometry();
    // Set volume as sensitive detector
    void set_sd();

    // Target material parameters
    struct TargetParams
    {
        double temperature = 300 * CLHEP::kelvin;
        double density = 1.0;  // In units of liquid hydrogen density
        double triton_fraction = 0.5;  // [mole fraction]
        double deuteron_fraction = 0.5;  // [mole fraction]
    } target_;

    // Liquid hydrogen density unit (4.25e22 atoms/cm3)
    static double constexpr liquid_hydrogen_density_
        = 4.25e22 / CLHEP::Avogadro * CLHEP::mole / CLHEP::cm3;
};
