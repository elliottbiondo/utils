//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RootIO.hh
//! \brief ROOT I/O interface.
//---------------------------------------------------------------------------//
#pragma once

#include <map>
#include <memory>
#include <string>

#include "G4appMacros.hh"
#include "RootData.hh"
#include "RootUniquePtr.hh"

class TFile;
class TTree;

//---------------------------------------------------------------------------//
/*!
 * ROOT I/O interface. It creates a singleton to manage data provided by
 * different Geant4 classes.
 *
 * Use \c RootIO::construct() to create the ROOT file at the beginning of the
 * simulation. Call \c RootIO::instance() to get access to the constructed
 * RootIO object from any class method. Writing and, at the end, closing the
 * TFile must be done by the user by invoking \c tfile_->Write() and
 * \c tfile_->Close() .
 *
 * \note
 * If `USE_ROOT=OFF`, `construct()` does not initialize the singleton. All
 * actions (run, event, tracking, step) check if the singleton is initialized.
 * If it's a nullptr, all actions will return before any I/O is called.
 */
class RootIO
{
  public:
    // Construct by initializing singleton and TFile
    static void construct(char const* root_filename);

    // Get singleton instance
    static RootIO* instance();

    // Clear event_ object after a TTree->Fill()
    void clear_event();

    // Clear track_ object after a TTree->Fill()
    void clear_track();

    // Set up ID for sensitive detector
    void add_sd(rootdata::SensDetGdml from_gdml);

    // Fill event TTree
    void fill_event_ttree();

    // Fill data limits TTree
    void fill_data_limits_ttree();

    // Store execution files in a separate TTree
    void store_performance_metrics(rootdata::ExecutionTime& exec_times);

    // Store sensitive detector names and their ids
    void store_sd_map();

    // Store input values for future reference
    void store_input();

    // Check if full MC data must be stored or not
    bool is_performance_run();

    // Write TFile
    void write_tfile();

  public:
    //!@{
    //! \name Type aliases
    using SensitiveDetectorMap = std::map<rootdata::SensDetGdml, unsigned int>;
    //!@}

    // TFile structure
    RootUP<TFile> tfile_;
    RootUP<TTree> ttree_event_;
    RootUP<TTree> ttree_data_limits_;

    // Temporary objects in memory written to the TFile at TTree->Fill()
    rootdata::Event event_;
    rootdata::Track track_;
    rootdata::DataLimits data_limits_;
    unsigned long steps_per_event_;

    // Map SD name/copy number to index in event_.sensitive_detectors
    SensitiveDetectorMap sdgdml_sensdetidx_;

  private:
    // Invoked by construct()
    RootIO(char const* root_filename);

  private:
    bool is_performance_run_;
};

//---------------------------------------------------------------------------//
#if !USE_ROOT
inline void RootIO::construct(char const*)
{
    // Do not initialize singleton
    return;
}

inline RootIO* RootIO::instance()
{
    return nullptr;
}

inline void RootIO::clear_event()
{
    return;
}

inline void RootIO::clear_track()
{
    return;
}

inline void RootIO::add_sd(rootdata::SensDetGdml)
{
    return;
}

inline void RootIO::fill_event_ttree()
{
    return;
}

inline void RootIO::fill_data_limits_ttree()
{
    return;
}

inline void RootIO::store_performance_metrics(rootdata::ExecutionTime&)
{
    return;
}

inline void RootIO::store_sd_map()
{
    return;
}

inline void RootIO::store_input()
{
    return;
}

inline bool RootIO::is_performance_run()
{
    return true;
}

inline void RootIO::write_tfile()
{
    return;
}
#endif
