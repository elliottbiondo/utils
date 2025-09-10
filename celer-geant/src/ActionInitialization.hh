//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/ActionInitialization.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4VUserActionInitialization.hh>

//---------------------------------------------------------------------------//
/*!
 * Initialize all user action classes, set up Celeritas offloading interface,
 * and assign the Celeritas' implementation of \c G4VTrackingManager to the
 * particles that should be offloaded.
 */
class ActionInitialization final : public G4VUserActionInitialization
{
  public:
    //! Construct empty
    ActionInitialization() = default;

    //! Initialize Celeritas offload interface on master thread
    void BuildForMaster() const final;

    //! Initialize user-actions and Celeritas offloading on worker threads
    void Build() const final;
};
