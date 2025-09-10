//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/RootIO.hh
//---------------------------------------------------------------------------//
#pragma once

#include <TFile.h>

#include "RootDataStore.hh"

//---------------------------------------------------------------------------//
/*!
 * Thread-local ROOT I/O manager singleton.
 *
 * This class stores a \c RootDataStore object, which keeps track of data for
 * sensitive detectors.
 *
 * The data from this singleton is mainly collected during the
 * \c SensitiveDetector::ProcessHits function call, but can be used in tandem
 * with \c G4UserEventAction . E.g. storing the total energy deposition in
 * a given sensitive detector can be done by accumulating the total energy
 * during \c ProcessHits  and the final tally written to a histogram at
 * \c G4UserEventAction::EndOfEventAction .
 */
class RootIO
{
  public:
    //! Return a thread-local singleton instance
    static RootIO* Instance();

    //! Get reference to thread-local Histogram data
    RootDataStore& Histograms() { return data_store_; }

    //! Store OutputRegistry diagnostics
    void StoreDiagnostics(std::string diagnostics);

    //! Write data to ROOT file and close it
    void Finalize();

  private:
    //// DATA ////

    TFile* file_;
    RootDataStore data_store_;

    //// HELPER FUNCTIONS ////

    // Construct with JSON input filename on worker thread
    RootIO();

    // ROOT TTree split level
    static constexpr short int SplitLevel() { return 99; }
};
