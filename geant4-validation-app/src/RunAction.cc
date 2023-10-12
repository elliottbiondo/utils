//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RunAction.cc
//---------------------------------------------------------------------------//
#include "RunAction.hh"

#include <G4RunManager.hh>

#include "JsonReader.hh"
#include "Celeritas.hh"
#include <accel/ExceptionConverter.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct by selecting RNG seed and verbosity.
 */
RunAction::RunAction() : G4UserRunAction(), root_io_(RootIO::instance())
{
    const auto& json = JsonReader::instance()->json();

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
void RunAction::BeginOfRunAction(const G4Run*)
{
    if (offload_)
    {
        celeritas::ExceptionConverter HandleExceptions{"celer0001"};
        if (G4Threading::IsMasterThread())
        {
            CELER_TRY_HANDLE(
                CelerSharedParams().Initialize(CelerSetupOptions()),
                HandleExceptions);
        }
        else
        {
            CELER_TRY_HANDLE(
                celeritas::SharedParams::InitializeWorker(CelerSetupOptions()),
                HandleExceptions);
        }

        if (G4Threading::IsWorkerThread()
            || !G4Threading::IsMultithreadedApplication())
        {
            CELER_TRY_HANDLE(CelerLocalTransporter().Initialize(
                                 CelerSetupOptions(), CelerSharedParams()),
                             HandleExceptions);
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Write data to the ROOT file and write file to disk.
 */
void RunAction::EndOfRunAction(const G4Run*)
{
    if (offload_)
    {
        celeritas::ExceptionConverter HandleExceptions{"celer0005"};

        if (CelerLocalTransporter())
        {
            CELER_TRY_HANDLE(CelerLocalTransporter().Finalize(),
                             HandleExceptions);
        }

        if (G4Threading::IsMasterThread())
        {
            CELER_TRY_HANDLE(CelerSharedParams().Finalize(), HandleExceptions);
        }
    }

    if (!root_io_)
    {
        return;
    }

    root_io_->fill_data_limits_ttree();
}
