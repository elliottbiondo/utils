//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/StackingAction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <vector>
#include <G4UserStackingAction.hh>

//---------------------------------------------------------------------------//
/*!
 * Classify any particle that should not be offloaded as \c fKill .
 *
 * \todo: Improve initial default list.
 */
class StackingAction : public G4UserStackingAction
{
  public:
    // Construct with JSON input data
    StackingAction();

    // Kill non-offloaded tracks
    G4ClassificationOfNewTrack ClassifyNewTrack(G4Track* const track);

  private:
    using PDG = int;
    std::vector<PDG> valid_pdgs_{11, -11, 22};
};
