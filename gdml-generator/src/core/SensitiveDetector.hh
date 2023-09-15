//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SimpleCMSSD.hh
//! \brief Sensitive detector class.
//---------------------------------------------------------------------------//
#pragma once

#include <G4VSensitiveDetector.hh>

//---------------------------------------------------------------------------//
/*!
 * Interface for sensitive detectors.
 */
class SensitiveDetector : public G4VSensitiveDetector
{
  public:
    // Construct with sensitive detector name
    SensitiveDetector(G4String sd_name);

    // Mandatory. Called at each step
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) final;
};
