//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file MucfBox.hh
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
class MucfBox : public G4VUserDetectorConstruction
{
  public:
    // Construct
    MucfBox();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;

    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;

  private:
    // Build mucf box target
    G4VPhysicalVolume* build_target_box();

    // Set volume as sensitive detector
    void set_sd_target_box();
};
