//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file gdml-gen.cc
//! \brief Export Geant4 test-problem geometries as GDML files.
//---------------------------------------------------------------------------//
#include <G4Version.hh>
#if defined(G4VERSION_NUMBER) && G4VERSION_NUMBER < 1100
#    define G4_V10 1
#else
#    define G4_V10 0
#endif

#if G4_V10
#    include <G4RunManager.hh>
#else
#    include <G4RunManagerFactory.hh>
#endif

#include <G4GDMLParser.hh>
#include <G4UImanager.hh>
#include "BoxDetector.hh"
#include "SimpleCMSDetector.hh"
#include "TestEm3Detector.hh"
#include "core/PhysicsList.hh"

enum class GeometryID
{
    box,
    simple_cms,             //!< Simple materials
    simple_cms_composite,   //!< Composite materials
    testem3,                //!< Simple materials
    testem3_composite,      //!< Composite materials
    testem3_flat,           //!< Simple materials, flat (for ORANGE)
    testem3_composite_flat, //!< Composite materials flat (for ORANGE)
};

//---------------------------------------------------------------------------//
/*!
 * Help message.
 */
void print_help(const char* argv)
{
    std::cout << "Usage:" << std::endl;
    std::cout << argv
              << " [geometry_id] [optional: prod_cuts_mm (default = 0.7 mm)]"
              << std::endl;
    std::cout << std::endl;
    std::cout << "Geometries:" << std::endl;
    std::cout << "0: Box" << std::endl;
    std::cout << "1: Simple CMS - simple materials" << std::endl;
    std::cout << "2: Simple CMS - composite materials" << std::endl;
    std::cout << "3: TestEm3 - simple materials" << std::endl;
    std::cout << "4: TestEm3 - composite materials" << std::endl;
    std::cout << "5: TestEm3 flat - simple materials, for ORANGE" << std::endl;
    std::cout << "6: TestEm3 flat - composite materials, for ORANGE"
              << std::endl;
}

//---------------------------------------------------------------------------//
/*!
 * Export and write GDML file to disk.
 */
void export_gdml(std::string const& gdml_filename)
{
    G4GDMLParser parser;
    parser.SetEnergyCutsExport(true);
    parser.SetSDExport(true);
    parser.SetOverlapCheck(true);
    parser.SetOutputFileOverwrite(true);
    parser.Write(gdml_filename,
                 G4TransportationManager::GetTransportationManager()
                     ->GetNavigatorForTracking()
                     ->GetWorldVolume()
                     ->GetLogicalVolume(),
                 true); // bool appends ptr address to name
}

//---------------------------------------------------------------------------//
/*
 * Generate GDML geometry files for benchmarking and validation.
 * For more details on existing geometries see the README.md.
 *
 * Usage:
 * ./gdm-gen [geometry_id] [optional: production_cuts_mm]
 */
int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 3)
    {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    // Create Run Manager
    std::unique_ptr<G4RunManager> run_manager;
#if G4_V10
    run_manager.reset(new G4RunManager());
#else
    run_manager.reset(
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial));
#endif

    // Load input parameters
    const auto   geometry_id = static_cast<GeometryID>(std::stoi(argv[1]));
    const double range_cuts  = (argc == 3) ? std::stod(argv[2]) : 0.7;

    using CMSType        = SimpleCMSDetector::MaterialType;
    using TestEm3MatType = TestEm3Detector::MaterialType;
    using TestEm3GeoType = TestEm3Detector::GeometryType;

    // Construct geometry and define gdml filename
    std::string gdml_filename;
    switch (geometry_id)
    {
        case GeometryID::box:
            run_manager->SetUserInitialization(new BoxDetector());
            gdml_filename = "box.gdml";
            break;
        case GeometryID::simple_cms:
            run_manager->SetUserInitialization(
                new SimpleCMSDetector(CMSType::simple));
            gdml_filename = "simple-cms.gdml";
            break;
        case GeometryID::simple_cms_composite:
            run_manager->SetUserInitialization(
                new SimpleCMSDetector(CMSType::composite));
            gdml_filename = "composite-simple-cms.gdml";
            break;
        case GeometryID::testem3:
            run_manager->SetUserInitialization(new TestEm3Detector(
                TestEm3MatType::simple, TestEm3GeoType::hierarchical));
            gdml_filename = "testem3.gdml";
            break;
        case GeometryID::testem3_composite:
            run_manager->SetUserInitialization(new TestEm3Detector(
                TestEm3MatType::composite, TestEm3GeoType::hierarchical));
            gdml_filename = "testem3-composite.gdml";
            break;
        case GeometryID::testem3_flat:
            run_manager->SetUserInitialization(new TestEm3Detector(
                TestEm3MatType::simple, TestEm3GeoType::flat));
            gdml_filename = "testem3-flat.gdml";
            break;
        case GeometryID::testem3_composite_flat:
            run_manager->SetUserInitialization(new TestEm3Detector(
                TestEm3MatType::composite, TestEm3GeoType::flat));
            gdml_filename = "testem3-flat-composite.gdml";
            break;
        default:
            std::cout << (int)geometry_id << " is an invalid geometry id."
                      << std::endl;
            return EXIT_FAILURE;
    }

    // Load phisics list, particle gun, and initialize run manager
    run_manager->SetUserInitialization(new PhysicsList(range_cuts));
    // run_manager->SetUserInitialization(new ActionInitialization());
    run_manager->Initialize();
    run_manager->RunInitialization();

    // Write GDML to disk
    export_gdml(gdml_filename);

    return EXIT_SUCCESS;
}
