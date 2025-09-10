//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/EventAction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserEventAction.hh>
#include <stddef.h>

//---------------------------------------------------------------------------//
/*!
 * Print step statistics at the end of every event.
 */
class EventAction final : public G4UserEventAction
{
  public:
    //! Construct and define progress logging
    EventAction();

    //! Begin of event user-action
    void BeginOfEventAction(G4Event const* event) final;

    //! End of event user-action
    void EndOfEventAction(G4Event const*) final;

  private:
    size_t log_progress_;
};
