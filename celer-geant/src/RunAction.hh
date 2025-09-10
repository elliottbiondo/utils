//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/RunAction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserRunAction.hh>

//---------------------------------------------------------------------------//
/*!
 * Manage Celeritas offloading interface at beginning/end of run.
 */
class RunAction : public G4UserRunAction
{
  public:
    //! Construct empty
    RunAction() = default;

    //! Initialize I/O and Celeritas offloading interface
    void BeginOfRunAction(G4Run const* run) final;

    //! Finalize I/O and Celeritas offloading interface
    void EndOfRunAction(G4Run const* run) final;
};
