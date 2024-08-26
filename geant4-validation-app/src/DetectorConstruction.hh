//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <G4GDMLParser.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    // Construct and parse gdml input file
    DetectorConstruction();

    // Construct geometry
    G4VPhysicalVolume* Construct() override;
    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() override;

  private:
    // Set simple cms volumes as sensitive detectors
    void set_sd();

  private:
    // Assigned by Constr() and released ownership at Construct()
    std::unique_ptr<G4VPhysicalVolume> phys_vol_world_;
    G4GDMLParser gdml_parser_;
};
