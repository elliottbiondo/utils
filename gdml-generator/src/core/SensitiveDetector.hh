//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SimpleCMSSD.hh
//! \brief Sensitive detector class.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4VSensitiveDetector.hh>

//---------------------------------------------------------------------------//
/*!
 * This class is only used to flag volumes as sensitive in the GDML file.
 */
class SensitiveDetector : public G4VSensitiveDetector
{
  public:
    // Construct with sensitive detector name
    SensitiveDetector(std::string sd_name);

    // Mandatory. Called at each step
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) final;
};
