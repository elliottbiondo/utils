//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/celer-geant.cc
//! \brief Celeritas-Geant4 offloading application
//---------------------------------------------------------------------------//
#include <iostream>
#include <memory>
#include <G4Electron.hh>
#include <G4Positron.hh>
#include <G4RunManagerFactory.hh>
#include <G4Threading.hh>
#include <G4UImanager.hh>
#include <accel/TrackingManagerConstructor.hh>
#include <accel/TrackingManagerIntegration.hh>
#include <celeritas/ext/EmPhysicsList.hh>
#include <corecel/io/Logger.hh>

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "JsonReader.hh"
#include "MakeCelerOptions.hh"
#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Run a Celeritas-Geant4 execution run for physics validation.
 *
 * See README for details.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        // Print help message
        std::cout << "Usage: " << argv[0] << " input.json" << std::endl;
        return EXIT_FAILURE;
    }

    // Load and verify input file
    JsonReader::Construct(argv[1]);
    auto const& json = JsonReader::Instance();

    JsonReader::Validate(json, "num_threads");
    auto const num_threads = json.at("num_threads").get<size_t>();
    CELER_VALIDATE(num_threads > 0, << "Number of threads must be positive");

    std::unique_ptr<G4RunManager> run_manager;
    run_manager.reset(
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT));
    run_manager->SetNumberOfThreads(num_threads);

    // Initialize Celeritas
    auto& tmi = celeritas::TrackingManagerIntegration::Instance();
    tmi.SetOptions(MakeCelerOptions());

    // Initialize physics with Celeritas offload
    using PhysicsOptions = celeritas::GeantPhysicsOptions;
    using MuonPhysicsOptions = celeritas::GeantMuonPhysicsOptions;
    auto phys_opts = PhysicsOptions::deactivated();
    phys_opts.muon = MuonPhysicsOptions{};
    phys_opts.muon.msc = celeritas::MscModelSelection::none;

    auto physics = std::make_unique<celeritas::EmPhysicsList>(phys_opts);
    physics->RegisterPhysics(new celeritas::TrackingManagerConstructor(&tmi));
    run_manager->SetUserInitialization(physics.release());

    // Initialize geometry and actions
    JsonReader::Validate(json, "geometry");
    run_manager->SetUserInitialization(
        new DetectorConstruction(json.at("geometry").get<std::string>()));
    run_manager->SetUserInitialization(new ActionInitialization());

    // Run events
    JsonReader::Validate(json, "particle_gun");
    auto const& json_pg = json.at("particle_gun");
    JsonReader::Validate(json_pg, "num_events");
    auto const num_events = json_pg.at("num_events").get<size_t>();
    CELER_VALIDATE(num_events, << "Number of events must be positive");
    run_manager->Initialize();
    run_manager->BeamOn(num_events);

    return EXIT_SUCCESS;
}
