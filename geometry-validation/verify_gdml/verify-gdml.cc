//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4-geo-validation-app.cc
//! \brief Geant4 volume validation app.
//---------------------------------------------------------------------------//
#include <iostream>
#include <string>
#include <vector>
#include <G4EmStandardPhysics.hh>
#include <G4GDMLParser.hh>
#include <G4RunManager.hh>
#include <G4VModularPhysicsList.hh>
#include <G4VPhysicalVolume.hh>

#include "src/DetectorConstruction.hh"
#include "src/GeometryStore.hh"

//---------------------------------------------------------------------------//
/*!
 * GDML parser to verify how GDML files are loaded in memory, including but not
 * limited to volume ids, material ids, replica numbers, and physical volume
 * copy numbers.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " geometry.gdml" << std::endl;

        return EXIT_FAILURE;
    }

    std::string gdml_filename(argv[1]);

    std::unique_ptr<DetectorConstruction> detector
        = std::make_unique<DetectorConstruction>(gdml_filename);
    const auto* phys_world_vol = detector->get_world_volume();

    // Select G4EmStandardPhysics because its easy
    std::unique_ptr<G4VModularPhysicsList> physics
        = std::make_unique<G4VModularPhysicsList>();
    physics->RegisterPhysics(new G4EmStandardPhysics(0));

    // Initialize run manager
    G4RunManager run_manager;
    run_manager.SetVerboseLevel(0);
    run_manager.SetUserInitialization(detector.release());
    run_manager.SetUserInitialization(physics.release());
    run_manager.Initialize();
    run_manager.RunInitialization();

    auto geometry_store = GeometryStore();
    geometry_store.save("cmshllhc-parse.md");

    return EXIT_SUCCESS;
}
