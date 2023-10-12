//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file EventAction.hh
//! \brief Event management.
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserEventAction.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Manage event execution.
 */
class EventAction : public G4UserEventAction
{
  public:
    // Construct by setting up I/O and verbosity options
    EventAction();

    // Mandatory begin of event function
    void BeginOfEventAction(const G4Event* event) override;
    // Mandatory end of event function
    void EndOfEventAction(const G4Event*) override;

  private:
    RootIO* root_io_;
    bool    offload_;
    bool    store_primaries_;
    bool    store_secondaries_;
};
