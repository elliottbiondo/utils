//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <string>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct detector geometry with GDML file.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    // Construct
    DetectorConstruction(std::string gdml_filename);

    // Construct geometry for a Geant4 simulation run
    G4VPhysicalVolume* Construct() override;

    // Get pointer to the physical world volume
    const G4VPhysicalVolume* get_world_volume() const;

  private:
    std::unique_ptr<G4VPhysicalVolume> world_phys_vol_;
};
