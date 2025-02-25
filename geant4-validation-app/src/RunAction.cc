//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RunAction.cc
//---------------------------------------------------------------------------//
#include "RunAction.hh"

#include <G4RunManager.hh>
#include <accel/UserActionIntegration.hh>

#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct by selecting RNG seed and verbosity.
 */
RunAction::RunAction() : G4UserRunAction(), root_io_(RootIO::instance())
{
    auto const& json = JsonReader::instance()->json();

    G4RunManager::GetRunManager()->SetVerboseLevel(
        json.at("verbosity").at("RunAction").get<int>());

    offload_ = json.at("simulation").at("offload").get<bool>();

    if (int n_evts = json.at("verbosity").at("PrintProgress").get<int>())
    {
        G4RunManager::GetRunManager()->SetPrintProgress(n_evts);
    }

    // TODO: Set RNG engine

    if (json.at("simulation").at("random_seed").get<bool>())
    {
        // Random seed set by the clock time
        CLHEP::HepRandom::setTheSeed(time(0));
    }
}

//---------------------------------------------------------------------------//
/*!
 * Begin of run actions.
 */
void RunAction::BeginOfRunAction(G4Run const* run)
{
    if (offload_)
    {
        celeritas::UserActionIntegration::Instance().BeginOfRunAction(run);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Write data to the ROOT file and write file to disk.
 */
void RunAction::EndOfRunAction(G4Run const* run)
{
    if (offload_)
    {
        celeritas::UserActionIntegration::Instance().EndOfRunAction(run);
    }

    if (!root_io_)
    {
        return;
    }

    root_io_->fill_data_limits_ttree();
}
