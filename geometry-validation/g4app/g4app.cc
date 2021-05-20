//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4-geo-validation-app.cc
//! \brief Geant4 volume validation app.
//---------------------------------------------------------------------------//
#include <iostream>
#include <vector>
#include <string>

#include <G4RunManager.hh>
#include <G4UImanager.hh>
#include <G4VPhysicalVolume.hh>
#include <G4GDMLParser.hh>
#include <G4EmStandardPhysics.hh>
#include <G4VModularPhysicsList.hh>

#include "src/DetectorConstruction.hh"
#include "src/PrimaryGeneratorAction.hh"
#include "src/GeometryStore.hh"

//---------------------------------------------------------------------------//
/*!
 * Volume validation app.
 *
 * Used to test agreement between different geometry inputs:
 * - Geant4 programmatic geometry
 * - Geant4 GDML Parser
 *
 * To be done: comparison with VecGeom GDML Parser
 */
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " [geometry enum] [bool is_gdml]"
                  << std::endl;
        std::cout << "Example: " << argv[0] << " 0 1" << std::endl;

        return EXIT_FAILURE;
    }

    if (std::atoi(argv[1]) > 3)
    {
        std::cout << "Max geometry enum option is 3" << std::endl;
        return EXIT_FAILURE;
    }

    Geometry selected_geometry = static_cast<Geometry>(std::atoi(argv[1]));
    bool     is_gdml           = static_cast<bool>(std::atoi(argv[2]));

    // Execute a minimal run to load material information
    G4RunManager run_manager;
    run_manager.SetVerboseLevel(0);

    std::unique_ptr<DetectorConstruction> detector
        = std::make_unique<DetectorConstruction>(selected_geometry, is_gdml);

    const auto  phys_world_volume_prog = detector->get_world_volume();
    std::string gdml_filename          = detector->get_gdml_filename();

    // Select G4EmStandardPhysics
    std::unique_ptr<G4VModularPhysicsList> physics
        = std::make_unique<G4VModularPhysicsList>();
    physics->RegisterPhysics(new G4EmStandardPhysics());

    // Initialize and run
    run_manager.SetUserInitialization(detector.release());
    run_manager.SetUserInitialization(physics.release());
    run_manager.SetUserAction(new PrimaryGeneratorAction());
    G4UImanager::GetUIpointer()->ApplyCommand("/run/initialize");
    run_manager.BeamOn(1);

    GeometryStore geo_tester;
    geo_tester(phys_world_volume_prog);

    std::string extension = is_gdml ? "_gdml.txt" : "_prog.txt";
    std::string txt_filename
        = gdml_filename.substr(0, gdml_filename.find_last_of('.')) + extension;

    geo_tester.save(txt_filename);
    std::cout << geo_tester.get_map() << std::endl;

    return EXIT_SUCCESS;
}
