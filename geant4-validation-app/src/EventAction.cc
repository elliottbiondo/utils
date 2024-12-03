//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file EventAction.cc
//---------------------------------------------------------------------------//
#include "EventAction.hh"

#include <algorithm>
#include <G4Event.hh>
#include <G4EventManager.hh>
#include <accel/ExceptionConverter.hh>

#include "Celeritas.hh"
#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*
 * Construct by setting up I/O and verbosity options.
 */
EventAction::EventAction() : G4UserEventAction(), root_io_(RootIO::instance())
{
    auto const& json = JsonReader::instance()->json();
    offload_ = json.at("simulation").at("offload").get<bool>();
    store_primaries_ = json.at("simulation").at("primary_info").get<bool>();
    store_secondaries_ = json.at("simulation").at("secondary_info").get<bool>();
    G4EventManager::GetEventManager()->SetVerboseLevel(
        json.at("verbosity").at("EventAction").get<int>());
}

//---------------------------------------------------------------------------//
/*
 * Clear event in ROOT I/O and set up any needed event information.
 */
void EventAction::BeginOfEventAction(G4Event const* event)
{
    if (offload_)
    {
        celeritas::ExceptionConverter call_g4exception{"celer0002"};
        CelerLocalTransporter().InitializeEvent(event->GetEventID());
    }

    if (!root_io_)
    {
        return;
    }

    root_io_->clear_event();
    root_io_->event_.id = event->GetEventID();
    root_io_->steps_per_event_ = 0;
}

//---------------------------------------------------------------------------//
/*
 * Fill ROOT I/O event TTree and store data limits.
 */
void EventAction::EndOfEventAction(G4Event const*)
{
    if (offload_)
    {
        celeritas::ExceptionConverter call_g4exception{"celer0004"};
        CELER_TRY_HANDLE(CelerLocalTransporter().Flush(), call_g4exception);
    }

    if (!root_io_)
    {
        return;
    }

    root_io_->fill_event_ttree();

    // Store data limits
    if (store_primaries_)
    {
        root_io_->data_limits_.max_num_primaries
            = std::max(root_io_->event_.primaries.size(),
                       root_io_->data_limits_.max_num_primaries);
    }

    if (store_secondaries_)
    {
        root_io_->data_limits_.max_num_secondaries
            = std::max(root_io_->event_.secondaries.size(),
                       root_io_->data_limits_.max_num_secondaries);
    }

    if (store_primaries_ || store_secondaries_)
    {
        root_io_->data_limits_.max_steps_per_event
            = std::max(root_io_->steps_per_event_,
                       root_io_->data_limits_.max_steps_per_event);
    }
}
