//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2025 UT-Battelle, LLC, and other Celeritas developers.
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

#include "Box.hh"
#include "FourSteelSlabs.hh"
#include "MucfBox.hh"
#include "MucfTestGeo.hh"
#include "OpticalBoxes.hh"
#include "OpticalPrism.hh"
#include "SegmentedSimpleCms.hh"
#include "SimpleCms.hh"
#include "SimpleLZ.hh"
#include "TestEm3.hh"
#include "ThinSlab.hh"
#include "core/PhysicsList.hh"

//---------------------------------------------------------------------------//
/*!
 * GDML selection enum.
 */
enum class GeometryID
{
    box,  //!< 500 m lead box ("infinite" medium)
    four_steel_slabs,  //!< Four stainless steel slabs in a vacuum
    simple_cms,  //!< Simple materials
    simple_cms_composite,  //!< Composite materials
    segmented_simple_cms,  //!< Segmented Simple CMS
    segmented_simple_cms_composite,  //!< Segmented Simple CMS, composite
                                     //!< mats
    testem3,  //!< Simple materials
    testem3_composite,  //!< Composite materials
    testem3_flat,  //!< Simple materials, flat (for ORANGE)
    testem3_composite_flat,  //!< Composite materials flat (for ORANGE)
    optical_boxes,  //!< Boxes with optical properties
    thin_slab,  //!< Single material thin slab for MSC validation
    simple_lz,  //!< Simplified model of the LUX-ZEPLIN detector array
    mucf_test_geo,  //!< Test geometry for muon-catalyzed fusion
    mucf_box,  //!< Muon-catalyzed fusion box target only
    optical_prism,  //!< Triangular prism with optical properties
    size_
};

//---------------------------------------------------------------------------//
/*!
 * Geometry description.
 */
constexpr char const* label(GeometryID id) noexcept
{
    using GID = GeometryID;

    switch (id)
    {
        case GID::box:
            return "Lead box";
            break;
        case GID::four_steel_slabs:
            return "Four steel slabs";
            break;
        case GID::simple_cms:
            return "Simple CMS - simple materials";
            break;
        case GID::simple_cms_composite:
            return "Simple CMS - composite materials";
            break;
        case GID::segmented_simple_cms:
            return "Segmented Simple CMS - simple materials";
            break;
        case GID::segmented_simple_cms_composite:
            return "Segmented Simple CMS - composite materials";
            break;
        case GID::testem3:
            return "TestEm3 - simple materials";
            break;
        case GID::testem3_composite:
            return "TestEm3 - composite materials";
            break;
        case GID::testem3_flat:
            return "TestEm3 flat - simple materials, for ORANGE";
            break;
        case GID::testem3_composite_flat:
            return "TestEm3 flat - composite materials, for ORANGE";
            break;
        case GID::optical_boxes:
            return "Optical boxes - composite material boxes with optical "
                   "properties";
            break;
        case GID::thin_slab:
            return "Thin Pb slab";
            break;
        case GID::simple_lz:
            return "Simplified LZ - top PMT array";
            break;
        case GID::mucf_test_geo:
            return "MuCF test geometry - dt target and neutron counters";
            break;
        case GID::mucf_box:
            return "MuCF box target only";
            break;
        case GID::optical_prism:
            return "Optical triangular prism";
            break;
        default:
            __builtin_unreachable();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Help message.
 */
void print_help(char const* argv)
{
    using std::cout;
    using std::endl;

    cout << "Usage:" << endl;
    cout << argv << " [geometry_id]" << endl;
    cout << endl;
    cout << "Geometries:" << endl;
    for (auto i = 0; i < static_cast<int>(GeometryID::size_); i++)
    {
        std::string divider = (i < 10) ? " : " : ": ";
        cout << i << divider << label(static_cast<GeometryID>(i)) << endl;
    }
    cout << endl;
    cout << "For geometries "
         << static_cast<int>(GeometryID::segmented_simple_cms) << " and "
         << static_cast<int>(GeometryID::segmented_simple_cms_composite) << ":"
         << endl;
    cout << "3 extra parameters are needed - [num_segments_r] "
            "[num_segments_z] [num_segments_theta]"
         << endl;
    cout << "For " << static_cast<int>(GeometryID::simple_lz) << ":" << endl;
    cout << "1 extra parameter is optional - [sqrt_num_pmts] " << endl;
}

//---------------------------------------------------------------------------//
/*!
 * Set up number of segments for SegmentedSimpleCms geometry.
 */
SegmentedSimpleCms::SegmentDefinition get_segments(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cout << "Missing arguments" << std::endl;
        std::cout << argv[0] << " " << argv[1] << " "
                  << "[num_segments_r] [num_segments_z] [num_segments_theta]"
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    SegmentedSimpleCms::SegmentDefinition def;
    def.num_r = std::stoi(argv[2]);
    def.num_z = std::stoi(argv[3]);
    def.num_theta = std::stoi(argv[4]);
    return def;
}

//---------------------------------------------------------------------------//
/*!
 * Export geometry to GDML.
 */
void export_gdml(std::string const& gdml_filename)
{
    G4GDMLParser parser;
    parser.SetEnergyCutsExport(false);
    parser.SetSDExport(true);
    parser.SetOverlapCheck(true);
    parser.SetOutputFileOverwrite(true);
    parser.Write(gdml_filename,
                 G4TransportationManager::GetTransportationManager()
                     ->GetNavigatorForTracking()
                     ->GetWorldVolume()
                     ->GetLogicalVolume(),
                 /* append ptr address = */ true);
}

//---------------------------------------------------------------------------//
/*!
 * Generate GDML geometry files for benchmarking and validation.
 * For more details on existing geometries see the README.md.
 *
 * Usage:
 * ./gdm-gen [geometry_id] [optional parameters]
 */
int main(int argc, char* argv[])
{
    if (argc != 2 && argc != 3 && argc != 5)
    {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    // Load input parameters
    auto const geometry_id = static_cast<GeometryID>(std::stoi(argv[1]));
    if (geometry_id >= GeometryID::size_)
    {
        std::cout << static_cast<int>(geometry_id)
                  << " is an invalid geometry id." << std::endl;
        return EXIT_FAILURE;
    }
    if (geometry_id != GeometryID::segmented_simple_cms
        && geometry_id != GeometryID::segmented_simple_cms_composite
        && geometry_id != GeometryID::simple_lz && argc != 2)
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

    using CMSType = SimpleCms::MaterialType;
    using SCMSType = SegmentedSimpleCms::MaterialType;
    using TestEm3MatType = TestEm3::MaterialType;
    using TestEm3GeoType = TestEm3::GeometryType;

    // Construct geometry and define gdml filename
    std::string gdml_filename;
    switch (geometry_id)
    {
        case GeometryID::box:
            run_manager->SetUserInitialization(new Box());
            gdml_filename = "box.gdml";
            break;

        case GeometryID::four_steel_slabs:
            run_manager->SetUserInitialization(new FourSteelSlabs());
            gdml_filename = "four-steel-slabs.gdml";
            break;

        case GeometryID::simple_cms:
            run_manager->SetUserInitialization(new SimpleCms(CMSType::simple));
            gdml_filename = "simple-cms.gdml";
            break;

        case GeometryID::simple_cms_composite:
            run_manager->SetUserInitialization(
                new SimpleCms(CMSType::composite));
            gdml_filename = "composite-simple-cms.gdml";
            break;

        case GeometryID::segmented_simple_cms:
            run_manager->SetUserInitialization(new SegmentedSimpleCms(
                SCMSType::simple, get_segments(argc, argv)));
            gdml_filename = "segmented-simple-cms.gdml";
            break;

        case GeometryID::segmented_simple_cms_composite:
            run_manager->SetUserInitialization(new SegmentedSimpleCms(
                SCMSType::composite, get_segments(argc, argv)));
            gdml_filename = "composite-segmented-simple-cms.gdml";
            break;

        case GeometryID::testem3:
            run_manager->SetUserInitialization(new TestEm3(
                TestEm3MatType::simple, TestEm3GeoType::hierarchical));
            gdml_filename = "testem3.gdml";
            break;

        case GeometryID::testem3_composite:
            run_manager->SetUserInitialization(new TestEm3(
                TestEm3MatType::composite, TestEm3GeoType::hierarchical));
            gdml_filename = "testem3-composite.gdml";
            break;

        case GeometryID::testem3_flat:
            run_manager->SetUserInitialization(
                new TestEm3(TestEm3MatType::simple, TestEm3GeoType::flat));
            gdml_filename = "testem3-flat.gdml";
            break;

        case GeometryID::testem3_composite_flat:
            run_manager->SetUserInitialization(
                new TestEm3(TestEm3MatType::composite, TestEm3GeoType::flat));
            gdml_filename = "testem3-flat-composite.gdml";
            break;

        case GeometryID::optical_boxes:
            run_manager->SetUserInitialization(new OpticalBoxes());
            gdml_filename = "optical.gdml";
            break;

        case GeometryID::thin_slab:
            run_manager->SetUserInitialization(new ThinSlab());
            gdml_filename = "thin-slab.gdml";
            break;

        case GeometryID::simple_lz:
            gdml_filename = "simple_lz.gdml";
            if (argc == 2)
            {
                run_manager->SetUserInitialization(new SimpleLZ());
                break;
            }
            else if (argc == 3)
            {
                int sqrt_num_pmts = std::atoi(argv[2]);
                if (sqrt_num_pmts < 1)
                {
                    std::cout
                        << "The sqrt_num_pmts parameter must be positive "
                        << std::endl;
                    return EXIT_FAILURE;
                }
                run_manager->SetUserInitialization(new SimpleLZ(sqrt_num_pmts));
                break;
            }
            else
            {
                std::cout
                    << "SimpleLZ requires either 0 or 1 additional arguments "
                    << std::endl;
                return EXIT_FAILURE;
            }

        case GeometryID::mucf_test_geo:
            run_manager->SetUserInitialization(new MucfTestGeo());
            gdml_filename = "mucf-test-geo.gdml";
            break;

        case GeometryID::mucf_box:
            run_manager->SetUserInitialization(new MucfBox());
            gdml_filename = "mucf-box.gdml";
            break;

        case GeometryID::optical_prism:
            run_manager->SetUserInitialization(new OpticalPrism());
            gdml_filename = "optical-prism.gdml";
            break;

        default:
            __builtin_unreachable();
    }

    // Initialize run manager and export GDML (TODO: setup user-input cuts)
    run_manager->SetUserInitialization(new PhysicsList(/* range_cuts = */ 0.7));
    run_manager->Initialize();
    run_manager->RunInitialization();
    export_gdml(gdml_filename);

    return EXIT_SUCCESS;
}
