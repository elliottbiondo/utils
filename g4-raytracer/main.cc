//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
#include <G4EmStandardPhysics.hh>
#include <G4RunManager.hh>
#include <G4UIExecutive.hh>
#include <G4UImanager.hh>
#include <G4VModularPhysicsList.hh>
#include <G4VisExecutive.hh>

#include "src/DetectorConstruction.hh"

//---------------------------------------------------------------------------//
/*!
 * Geant4 raytracer app.
 * Loads and raytrace a GDML geometry. Visualization setup is managed via a
 * visualization macro.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        // Print help message
        std::cout << "Usage: " << argv[0] << " input.gdml" << std::endl;
        return EXIT_FAILURE;
    }

    G4RunManager run_manager;
    run_manager.SetVerboseLevel(1);  // Print minimal information about the run

    // Initialize physics and geometry
    std::unique_ptr<G4VModularPhysicsList> physics
        = std::make_unique<G4VModularPhysicsList>();
    physics->RegisterPhysics(new G4EmStandardPhysics(/* verbosity = */ 0));
    run_manager.SetUserInitialization(physics.release());
    run_manager.SetUserInitialization(
        new DetectorConstruction(std::string(argv[1])));
    run_manager.Initialize();

    // UI
    auto user_interface = new G4UIExecutive(argc, argv);
    G4VisExecutive* vis_manager = new G4VisExecutive();
    vis_manager->Initialize();

    auto ui_manager = G4UImanager::GetUIpointer();
    ui_manager->SetVerboseLevel(0);

    std::string vis_macro = "/control/execute vis.mac";
    ui_manager->ApplyCommand(vis_macro);

    user_interface->SessionStart();

    return EXIT_SUCCESS;
}
