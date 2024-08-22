//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SimpleCMSSD.cc
//---------------------------------------------------------------------------//
#include "SensitiveDetector.hh"

#include <G4SDManager.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct with sensitive detector name.
 */
SensitiveDetector::SensitiveDetector(std::string sd_name)
    : G4VSensitiveDetector(sd_name)
{
}

//---------------------------------------------------------------------------//
/*!
 * Mandatory function called at each step.
 */
G4bool SensitiveDetector::ProcessHits(G4Step*, G4TouchableHistory*)
{
    return false;
}
