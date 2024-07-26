//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <string>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct detector geometry.
 */
class DetectorConstruction final : public G4VUserDetectorConstruction
{
  public:
    // Construct with GDML filename
    DetectorConstruction(std::string input_gdml);

    // Construct world volume
    G4VPhysicalVolume* Construct() final;

  private:
    std::unique_ptr<G4VPhysicalVolume> phys_vol_world_;
};
