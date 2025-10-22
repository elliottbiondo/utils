//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/EventAction.cc
//---------------------------------------------------------------------------//
#include "EventAction.hh"

#include <G4Event.hh>
#include <RootIO.hh>
#include <corecel/io/Logger.hh>

#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct thread-local event action and set up event logging.
 */
EventAction::EventAction() : G4UserEventAction()
{
    auto json = JsonReader::Instance();
    log_progress_ = (json.contains("log_progress"))
                        ? json.at("log_progress").get<size_t>()
                        : 1;
}

//---------------------------------------------------------------------------//
/*!
 * Thread-local begin of event action.
 */
void EventAction::BeginOfEventAction(G4Event const* event)
{
    if (auto const id = event->GetEventID(); id % log_progress_ == 0)
    {
        CELER_LOG_LOCAL(status) << "Begin event " << id;
    }

    // Reset energy deposition for this event
    auto& sd_store = RootIO::Instance()->Data();
    for (auto& [ids, data] : sd_store.Map())
    {
        data.total_edep = 0;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Thread-local end of event action.
 */
void EventAction::EndOfEventAction(G4Event const* event)
{
    // Fill histograms with total energy deposited in each SD
    auto& sd_store = RootIO::Instance()->Data();
    for (auto& [ids, data] : sd_store.Map())
    {
        data.total_energy_dep.Fill(data.total_edep);
    }
}
