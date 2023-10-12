//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RunAction.hh
//! \brief Manage the simulation run.
//---------------------------------------------------------------------------//
#pragma once

#include <G4Run.hh>
#include <G4UserRunAction.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Manage simulation run.
 * Write the output ROOT file created by RootIO to disk.
 */
class RunAction : public G4UserRunAction
{
  public:
    // Construct by selecting RNG seed and set verbosity
    RunAction();

    // Mandatory begin of run action
    void BeginOfRunAction(const G4Run*) override;

    // Mandatory end of run action
    void EndOfRunAction(const G4Run*) override;

  private:
    RootIO* root_io_;
    bool    offload_;
};
