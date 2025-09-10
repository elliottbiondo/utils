//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/RunAction.cc
//---------------------------------------------------------------------------//
#include "RunAction.hh"

#include <G4Threading.hh>
#include <accel/ExceptionConverter.hh>
#include <accel/TrackingManagerIntegration.hh>
#include <celeritas/global/CoreParams.hh>
#include <corecel/io/Logger.hh>
#include <corecel/io/OutputRegistry.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Initialize master and worker threads in Celeritas.
 */
void RunAction::BeginOfRunAction(G4Run const* run)
{
    CELER_LOG_LOCAL(status) << "Begin of run action";
    celeritas::TrackingManagerIntegration::Instance().BeginOfRunAction(run);

    if (G4Threading::IsWorkerThread())
    {
        // Construct thread-local ROOT I/O
        // Initialization at begin of run ensures valid geometry and SD data
        // celeritas::ExceptionConverter avoids Geant4 not throwing exceptions
        CELER_TRY_HANDLE(RootIO::Instance(),
                         celeritas::ExceptionConverter{"celer-geant."
                                                       "beginrun"});
    }
}

//---------------------------------------------------------------------------//
/*!
 * Write thread-local ROOT file and return Celeritas to an invalid state.
 */
void RunAction::EndOfRunAction(G4Run const* run)
{
    using Mode = celeritas::OffloadMode;

    auto& tmi = celeritas::TrackingManagerIntegration::Instance();
    if (G4Threading::IsWorkerThread())
    {
        auto* rio = RootIO::Instance();
        if (tmi.GetMode() == Mode::enabled)
        {
            // Write Celeritas diagnostics to ROOT file
            std::ostringstream diagnostics;
            tmi.GetParams().output_reg()->output(&diagnostics);
            rio->StoreDiagnostics(diagnostics.str());
        }
        // Write and close ROOT output
        rio->Finalize();
    }
    // Return Celeritas to an invalid state
    tmi.EndOfRunAction(run);
}
