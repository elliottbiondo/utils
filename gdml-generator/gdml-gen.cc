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
#include <stdlib.h>

#include "BoxDetector.hh"
#include "SegmentedSimpleCmsDetector.hh"
#include "SimpleCmsDetector.hh"
#include "TestEm3Detector.hh"
#include "core/PhysicsList.hh"

enum class GeometryID
{
    box,
    simple_cms,  //!< Simple materials
    simple_cms_composite,  //!< Composite materials
    segmented_simple_cms,  //!< Segmented Simple CMS
    segmented_simple_cms_composite,  //!< Segmented Simple CMS, composite mats
    testem3,  //!< Simple materials
    testem3_composite,  //!< Composite materials
    testem3_flat,  //!< Simple materials, flat (for ORANGE)
    testem3_composite_flat,  //!< Composite materials flat (for ORANGE)
    size_
};

//---------------------------------------------------------------------------//
/*!
 * Help message.
 */
void print_help(char const* argv)
{
    std::cout << "Usage:" << std::endl;
    std::cout << argv << " [geometry_id]" << std::endl;
    std::cout << std::endl;
    std::cout << "Geometries:" << std::endl;
    std::cout << "0: Box" << std::endl;
    std::cout << "1: Simple CMS - simple materials" << std::endl;
    std::cout << "2: Simple CMS - composite materials" << std::endl;
    std::cout << "3: Segmented Simple CMS - simple materials" << std::endl;
    std::cout << "4: Segmented Simple CMS - composite materials" << std::endl;
    std::cout << "5: TestEm3 - simple materials" << std::endl;
    std::cout << "6: TestEm3 - composite materials" << std::endl;
    std::cout << "7: TestEm3 flat - simple materials, for ORANGE" << std::endl;
    std::cout << "8: TestEm3 flat - composite materials, for ORANGE"
              << std::endl;
    std::cout << std::endl;
    std::cout << "For Geometries 3 and 4:" << std::endl;
    std::cout << "3 extra parameters are needed - [num_segments_r] "
                 "[num_segments_theta] [num_segments_z]"
              << std::endl;
}

//---------------------------------------------------------------------------//
/*!
 * Set up number of segments for SegmentedSimpleCmsDetector geometry.
 */
SegmentedSimpleCmsDetector::SegmentDefinition
get_segments(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cout << "Missing arguments" << std::endl;
        std::cout << argv[0] << " " << argv[1] << " "
                  << "[num_segments_r] [num_segments_theta] [num_segments_z]"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    SegmentedSimpleCmsDetector::SegmentDefinition def;
    def.num_r = std::stoi(argv[2]);
    def.num_theta = std::stoi(argv[3]);
    def.num_z = std::stoi(argv[4]);
    return def;
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
                 true);  // bool appends ptr address to name
}

//---------------------------------------------------------------------------//
/*!
 * Generate GDML geometry files for benchmarking and validation.
 * For more details on existing geometries see the README.md.
 *
 * Usage:
 * ./gdm-gen [geometry_id] [optional: production_cuts_mm]
 *
 * \note
 * Temporarily disabling production cuts input until improve Segmented Simple
 * CMS input.
 */
int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 5)
    {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    // Load input parameters
    auto const geometry_id = static_cast<GeometryID>(std::stoi(argv[1]));
    double const range_cuts = 0.7;
    // double const range_cuts = (argc == 3) ? std::stod(argv[2]) : 0.7;

    if (geometry_id != GeometryID::segmented_simple_cms
        && geometry_id != GeometryID::segmented_simple_cms_composite
        && argc != 2)
    {
        std::cout << "Wrong number of arguments" << std::endl;
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

    using CMSType = SimpleCmsDetector::MaterialType;
    using SCMSType = SegmentedSimpleCmsDetector::MaterialType;
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
                new SimpleCmsDetector(CMSType::simple));
            gdml_filename = "simple-cms.gdml";
            break;

        case GeometryID::simple_cms_composite:
            run_manager->SetUserInitialization(
                new SimpleCmsDetector(CMSType::composite));
            gdml_filename = "composite-simple-cms.gdml";
            break;

        case GeometryID::segmented_simple_cms:
            run_manager->SetUserInitialization(new SegmentedSimpleCmsDetector(
                SCMSType::simple, get_segments(argc, argv)));
            gdml_filename = "segmented-simple-cms.gdml";
            break;

        case GeometryID::segmented_simple_cms_composite:
            run_manager->SetUserInitialization(new SegmentedSimpleCmsDetector(
                SCMSType::composite, get_segments(argc, argv)));
            gdml_filename = "composite-segmented-simple-cms.gdml";
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

    // Load phisics list and initialize run manager
    run_manager->SetUserInitialization(new PhysicsList(range_cuts));
    run_manager->Initialize();
    run_manager->RunInitialization();

    // Write GDML to disk
    export_gdml(gdml_filename);

    return EXIT_SUCCESS;
}
