//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SteppingAction.hh
//! \brief Collect step information.
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserSteppingAction.hh>
#include <G4Step.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Retrieve particle step data and save it to the root file.
 */
class SteppingAction : public G4UserSteppingAction
{
  public:
    // Construct and set up I/O options.
    SteppingAction();

    // Called at every step
    void UserSteppingAction(const G4Step* step) override;

  private:
    void store_track_data(const G4Step* step);
    void store_step_data(const G4Step* step);

  private:
    RootIO* root_io_;
    bool    store_step_;
    bool    store_primary_;
    bool    store_secondary_;
};
