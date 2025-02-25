//----------------------------------*-C++-*----------------------------------//
// Copyright 2023-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Geant4Run.hh
//! \brief Geant4 execution manager.
//---------------------------------------------------------------------------//
#pragma once

#include <fstream>
#include <memory>
#include <accel/SetupOptions.hh>

#include "G4appMacros.hh"
#include "JsonReader.hh"

#if G4_V10
#    if USE_MT
#        include <G4MTRunManager.hh>
#    else
#        include <G4RunManager.hh>
#    endif
#else
#    include <G4RunManagerFactory.hh>
#endif
#include <G4TransportationManager.hh>

class G4UIExecutive;

//---------------------------------------------------------------------------//
/*!
 * Manage the Geant4 execution. JsonReader and HepMC3Reader (if used)
 * singletons must be constructed before initializing this class.
 *
 * At construction the G4RunManager is created according to compile-time
 * conditions (G4 version and if MT is enabled), all user actions
 * (ActionInitialization, PhysicsList, and so on) are invoked, and the
 * G4RunManager is initialized.
 */
class Geant4Run
{
  public:
    // Construct based on compile-time and user-input options
    Geant4Run();

    // Run beam on and (optional) open GUI
    void beam_on();

    // Get number of events simulated
    int num_events() { return num_events_; }

    // Get copy of world volume
    G4VPhysicalVolume* world_volume();

  private:
    // Set all user initialization classes and initialize run manager
    void initialize();

    // Initialize visualization manager
    void init_vis_manager();

    // Get number of threads
    int num_threads();

    // Construct Celeritas options
    celeritas::SetupOptions celeritas_options();

  private:
#if G4_V10 && USE_MT
    // Multithreaded Geant4 v10
    using RunManager = G4MTRunManager;
#else
    // Singlethreaded Geant4 v10 OR v11 (both single and MT)
    using RunManager = G4RunManager;
#endif

    std::unique_ptr<RunManager> run_manager_;
    nlohmann::json json_;
    G4UIExecutive* qt_interface_{nullptr};  // Owned by RunManager
    int num_events_;
};
