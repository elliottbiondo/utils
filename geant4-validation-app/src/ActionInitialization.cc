//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file ActionInitialization.cc
//---------------------------------------------------------------------------//
#include "ActionInitialization.hh"

#include <accel/UserActionIntegration.hh>

#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct by defining if Celeritas offloading is enabled.
 */
ActionInitialization::ActionInitialization()
    : G4VUserActionInitialization()
    , offload_(JsonReader::instance()
                   ->json()
                   .at("simulation")
                   .at("offload")
                   .get<bool>())
{
}

//---------------------------------------------------------------------------//
/*!
 * Build master thread and initialize run.
 */
void ActionInitialization::BuildForMaster() const
{
    if (offload_)
    {
        celeritas::UserActionIntegration::Instance().BuildForMaster();
    }
    SetUserAction(new RunAction());
}

//---------------------------------------------------------------------------//
/*!
 * Invoke SetUserAction() classes on worker threads.
 */
void ActionInitialization::Build() const
{
    if (offload_)
    {
        celeritas::UserActionIntegration::Instance().Build();
    }

    SetUserAction(new RunAction());
    SetUserAction(new PrimaryGeneratorAction());
    SetUserAction(new EventAction());
    SetUserAction(new TrackingAction());
    SetUserAction(new SteppingAction());
}
