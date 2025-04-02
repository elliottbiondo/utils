//----------------------------------*-C++-*----------------------------------//
// Copyright 2023-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Geant4Run.cc
//---------------------------------------------------------------------------//
#include "Geant4Run.hh"

#include <iostream>
#include <G4UIExecutive.hh>
#include <G4UImanager.hh>
#include <G4VisExecutive.hh>
#include <accel/AlongStepFactory.hh>
#include <accel/UserActionIntegration.hh>

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "HepMC3Reader.hh"
#include "PhysicsList.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct based on compile-time macros and user-input options. JsonReader
 * and HepMC3Reader (if used) singletons must constructed already.
 */
Geant4Run::Geant4Run()
{
    // Construct json
    json_ = JsonReader::instance()->json();
    auto const& json_sim = json_.at("simulation");
    std::string const hepmc3_input = json_sim.at("hepmc3").get<std::string>();

    // Fetch correct number of events
    num_events_
        = (hepmc3_input.empty())
              ? json_sim.at("particle_gun").at("events").get<unsigned long>()
              : HepMC3Reader::instance()->number_of_events();

    if (json_sim.at("offload").get<bool>())
    {
        // Initialize Celeritas interface
        celeritas::UserActionIntegration::Instance().SetOptions(
            this->celeritas_options());
    }

    // Construct run manager
#if G4_V10
    // Geant4 v10.x.x
#    if USE_MT
    run_manager_.reset(new G4MTRunManager());  // Multithread
#    else
    run_manager_.reset(new G4RunManager());  // Singlethread
#    endif

#else
    // Geant4 v11.x.x
    run_manager_.reset(G4RunManagerFactory::CreateRunManager(
        USE_MT ? G4RunManagerType::MT : G4RunManagerType::Serial));
#endif

    // Set verbosity
    run_manager_->SetVerboseLevel(
        json_.at("verbosity").at("RunManager").get<int>());

    if (USE_MT && json_sim.at("performance_run").get<bool>())
    {
        // Set correct number of cores
        run_manager_->SetNumberOfThreads(this->num_threads());
    }

    // Initialize all user actions and run manager
    this->initialize();

    if (USE_QT && json_.at("GUI").get<bool>())
    {
        this->init_vis_manager();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Execute \c run_manager->BeamOn(n) and open Qt interface if `"GUI" = true`.
 */
void Geant4Run::beam_on()
{
    // Run events
    run_manager_->BeamOn(num_events_);

    if (qt_interface_)
    {
        // Open GUI
        qt_interface_->SessionStart();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Get copy of world volume for cases when it needs to be passed to Celeritas'
 * classes after Geant4Run goes out of scope.
 */
G4VPhysicalVolume* Geant4Run::world_volume()
{
    return G4TransportationManager::GetTransportationManager()
        ->GetNavigatorForTracking()
        ->GetWorldVolume();
}

//---------------------------------------------------------------------------//
/*!
 * Initialize all user actions and initialize run manager.
 */
void Geant4Run::initialize()
{
    run_manager_->SetUserInitialization(new DetectorConstruction());
    run_manager_->SetUserInitialization(new PhysicsList());
    run_manager_->SetUserInitialization(new ActionInitialization());
    run_manager_->Initialize();
}

//---------------------------------------------------------------------------//
/*!
 * Initialize user interface.
 */
void Geant4Run::init_vis_manager()
{
    char* argv[] = {};
    qt_interface_ = new G4UIExecutive(1, argv);
    auto vis_manager = new G4VisExecutive();
    vis_manager->Initialize();

    G4UImanager* ui_manager = G4UImanager::GetUIpointer();
    std::string vis_macro = "/control/execute "
                            + json_.at("vis_macro").get<std::string>();
    ui_manager->ApplyCommand(vis_macro);
}

//---------------------------------------------------------------------------//
/*!
 * Get requested number of threads.
 */
int Geant4Run::num_threads()
{
    // Set up number of cores
    auto const& json_sim = json_.at("simulation");
    int const num_threads = json_sim.at("num_threads").get<int>();
    int const num_cores = G4Threading::G4GetNumberOfCores();

    if (num_threads > num_cores)
    {
        // Number of threads exceeds available cores
        std::cout << "\nWARNING: " << num_threads
                  << " requested threads exceeds number of cores ("
                  << num_cores << ")." << std::endl;
        std::cout.flush();
    }

    return num_threads;
}

//---------------------------------------------------------------------------//
/*!
 * Celeritas run-time options.
 *
 * TODO: add options to input.json.
 */
celeritas::SetupOptions Geant4Run::celeritas_options()
{
    celeritas::SetupOptions so;
    so.max_num_tracks = 1024;
    so.initializer_capacity = 1024;
    so.secondary_stack_factor = 2.0;
    so.ignore_processes = {"CoulombScat", "Rayl"};  // Ignored processes

    // Set along-step factory with zero field
    so.make_along_step = celeritas::UniformAlongStepFactory();

    // Save diagnostic information
    so.output_file = "celeritas-diagnostic.json";
    return so;
}
