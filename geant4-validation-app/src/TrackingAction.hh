//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TrackingAction.hh
//! \brief Manage track information.
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserTrackingAction.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Process track information.
 */
class TrackingAction : public G4UserTrackingAction
{
  public:
    // Construct and set up ROOT I/O options
    TrackingAction();

    // Clear track object before the next track
    void PreUserTrackingAction(const G4Track*) override;

    // Write track information to the ROOT file at the end of tracking
    void PostUserTrackingAction(const G4Track* track) override;

  private:
    RootIO* root_io_;
    bool    offload_;
    bool    store_primaries_;
    bool    store_secondaries_;
};
