//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SensitiveDetector.hh
//! \brief Sensitive detector class.
//---------------------------------------------------------------------------//
#pragma once

#include <G4VSensitiveDetector.hh>
#include <vector>
#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Interface for sensitive detectors.
 */
class SensitiveDetector : public G4VSensitiveDetector
{
  public:
    // Construct with sensitive detector name
    SensitiveDetector(G4String sd_name, G4LogicalVolume* logical_volume);

    // Optional. Called at the beginning of each event
    void Initialize(G4HCofThisEvent* hit_col_of_evt) override;

    // Mandatory. Called at each step
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;

    // Optional. Called at the end of every event
    void EndOfEvent(G4HCofThisEvent*) override;

  private:
    RootIO*     root_io_;
    std::string sd_name_;
};
