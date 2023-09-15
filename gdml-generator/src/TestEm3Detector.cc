//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TestEm3Detector.cc
//---------------------------------------------------------------------------//
#include "TestEm3Detector.hh"

#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4VPhysicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with material and geometry options.
 */
TestEm3Detector::TestEm3Detector(MaterialType material_type,
                                 GeometryType geometry_type)
    : material_type_(material_type), geometry_type_(geometry_type)
{
}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* TestEm3Detector::Construct()
{
    switch (geometry_type_)
    {
        case GeometryType::hierarchical:
            phys_vol_world_.reset(this->create_testem3());
            break;
        case GeometryType::flat:
            phys_vol_world_.reset(this->create_testem3_flat());
            break;
        default:
            __builtin_unreachable();
    }

    return phys_vol_world_.release();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors and magnetic field.
 */
void TestEm3Detector::ConstructSDandField()
{
    if (geometry_type_ == GeometryType::hierarchical)
    {
        this->set_sd();
    }
    // TODO: Add magnetic field
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Build TestEm3 materials.
 */
TestEm3Detector::MaterialList TestEm3Detector::load_materials()
{
    const auto nist = G4NistManager::Instance();

    MaterialList list;
    list.world = nist->FindOrBuildMaterial("G4_Galactic");
    list.world->SetName("vacuum");
    list.absorber = nist->FindOrBuildMaterial("G4_lAr");
    list.absorber->SetName("lAr");

    switch (material_type_)
    {
        case MaterialType::simple:
            list.gap = nist->FindOrBuildMaterial("G4_Pb");
            list.gap->SetName("Pb");
            break;
        case MaterialType::composite:
            list.gap = nist->FindOrBuildMaterial("G4_PbWO4");
            list.gap->SetName("PbWO4");
            break;
        default:
            __builtin_unreachable();
    }

    return list;
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition taken from AdePT's repository.
 * See github.com/apt-sim/AdePT/tree/master/examples/TestEm3
 *
 * \note
 * - Code changes are kept ALARA.
 * - The original example code sets up 3 materials in G4 just to calculate
 *   their material/production cut couples and manually assigns these material
 *   IDs to VecGeom volumes.
 * - AdePT's example seems to have mistakenly inverted gap/absorber materials.
 * - AdePT defines G4EmParameters::SetMscRangeFactor(0.06) here. We don't.
 */
G4VPhysicalVolume* TestEm3Detector::create_testem3()
{
    const unsigned int num_layers        = 50;
    const double       CalorSizeYZ       = 40 * cm;
    const double       GapThickness      = 2.3 * mm;
    const double       AbsorberThickness = 5.7 * mm;

    const double LayerThickness = GapThickness + AbsorberThickness;
    const double CalorThickness = num_layers * LayerThickness;
    const double WorldSizeX     = 1.2 * CalorThickness;
    const double WorldSizeYZ    = 1.2 * CalorSizeYZ;

    // Create materials
    const auto materials = load_materials();

    // Define a world
    G4Box* worldBox = new G4Box(
        "world", 0.5 * WorldSizeX, 0.5 * WorldSizeYZ, 0.5 * WorldSizeYZ);
    G4LogicalVolume* worldLog
        = new G4LogicalVolume(worldBox, materials.world, "world");
    G4PVPlacement* worldPlaced = new G4PVPlacement(
        nullptr, G4ThreeVector(), worldLog, "world_pv", nullptr, false, false);

    // Define calorimeter
    auto calorBox = new G4Box("calorimeterBox",
                              0.5 * CalorThickness,
                              0.5 * CalorSizeYZ,
                              0.5 * CalorSizeYZ);

    auto calorLog
        = new G4LogicalVolume(calorBox, materials.world, "Calorimeter");

    new G4PVPlacement(nullptr,
                      G4ThreeVector(),
                      calorLog,
                      "calorimeter_pv",
                      worldLog,
                      false,
                      0,
                      false);

    // Layers
    auto layerBox = new G4Box(
        "layerBox", 0.5 * LayerThickness, 0.5 * CalorSizeYZ, 0.5 * CalorSizeYZ);

    // Absorbers
    auto gapBox = new G4Box(
        "gapBox", 0.5 * GapThickness, 0.5 * CalorSizeYZ, 0.5 * CalorSizeYZ);

    auto          gapLogic = new G4LogicalVolume(gapBox, materials.gap, "Gap");
    G4ThreeVector gapPlacement(
        -0.5 * LayerThickness + 0.5 * GapThickness, 0, 0);

    auto absorberBox = new G4Box("absorberBox",
                                 0.5 * AbsorberThickness,
                                 0.5 * CalorSizeYZ,
                                 0.5 * CalorSizeYZ);
    auto absorberLogic
        = new G4LogicalVolume(absorberBox, materials.absorber, "Absorber");
    G4ThreeVector absorberPlacement(
        0.5 * LayerThickness - 0.5 * AbsorberThickness, 0, 0);

    // Create a new LogicalVolume per layer, we need unique IDs for scoring.
    double xCenter = -0.5 * CalorThickness + 0.5 * LayerThickness;

    for (int i = 0; i < num_layers; i++)
    {
        std::string layerName = "Layer_" + std::to_string(i);
        auto        layer_lv
            = new G4LogicalVolume(layerBox, materials.world, layerName);

        G4ThreeVector placement(xCenter, 0, 0);

        new G4PVPlacement(
            nullptr, placement, layer_lv, "layer_pv", calorLog, false, 0, false);

        new G4PVPlacement(
            nullptr, gapPlacement, gapLogic, "gap_pv", layer_lv, false, i, false);

        new G4PVPlacement(nullptr,
                          absorberPlacement,
                          absorberLogic,
                          "absorber_pv",
                          layer_lv,
                          false,
                          i,
                          false);

        xCenter += LayerThickness;
    }

    return worldPlaced;
}

//---------------------------------------------------------------------------//
/*!
 * Set up TestEm3 sensitive detectors.
 */
void TestEm3Detector::set_sd()
{
    auto sd_gap = new SensitiveDetector("sd_gap");
    auto sd_abs = new SensitiveDetector("sd_absorber");

    G4SDManager::GetSDMpointer()->AddNewDetector(sd_gap);
    G4SDManager::GetSDMpointer()->AddNewDetector(sd_abs);
    G4VUserDetectorConstruction::SetSensitiveDetector("Gap", sd_gap);
    G4VUserDetectorConstruction::SetSensitiveDetector("Absorber", sd_abs);
}

//---------------------------------------------------------------------------//
/*!
 * Flatten out the original TestEm3 geometry so that the exported gdml is
 * easily ported to ORANGE.
 *
 * To be used *just* to produce a flattened gdml file.
 * Sensitive detector data is not exported and any run will fail to store any
 * scored results.
 *
 * DO NOT USE IN A COMPARISON RUN.
 */
G4VPhysicalVolume* TestEm3Detector::create_testem3_flat()
{
    const unsigned int num_layers        = 50;
    const double       CalorSizeYZ       = 40 * cm;
    const double       GapThickness      = 2.3 * mm;
    const double       AbsorberThickness = 5.7 * mm;

    const double LayerThickness = GapThickness + AbsorberThickness;
    const double CalorThickness = num_layers * LayerThickness;
    const double WorldSizeX     = 1.2 * CalorThickness;
    const double WorldSizeYZ    = 1.2 * CalorSizeYZ;

    // Create materials
    const auto materials = load_materials();

    // World definition and placement
    G4Box* worldBox = new G4Box(
        "world_shape", 0.5 * WorldSizeX, 0.5 * WorldSizeYZ, 0.5 * WorldSizeYZ);
    G4LogicalVolume* worldLog
        = new G4LogicalVolume(worldBox, materials.world, "world_lv");
    G4PVPlacement* worldPlaced = new G4PVPlacement(
        nullptr, G4ThreeVector(), worldLog, "world", nullptr, false, false);

    // Gaps and absorbers
    auto gapBox = new G4Box(
        "gap_shape", 0.5 * GapThickness, 0.5 * CalorSizeYZ, 0.5 * CalorSizeYZ);
    auto absorberBox = new G4Box("absorber_shape",
                                 0.5 * AbsorberThickness,
                                 0.5 * CalorSizeYZ,
                                 0.5 * CalorSizeYZ);

    double xCenter = -0.5 * CalorThickness + 0.5 * LayerThickness;

    for (int i = 0; i < num_layers; i++)
    {
        std::string absorber_name = "absorber_" + std::to_string(i);
        std::string gap_name      = "gap_" + std::to_string(i);

        auto gapLogic = new G4LogicalVolume(gapBox, materials.gap, gap_name);
        auto absorberLogic = new G4LogicalVolume(
            absorberBox, materials.absorber, absorber_name);

        G4ThreeVector gapPlacement(
            xCenter - 0.5 * LayerThickness + 0.5 * GapThickness, 0, 0);
        G4ThreeVector absorberPlacement(
            xCenter + 0.5 * LayerThickness - 0.5 * AbsorberThickness, 0, 0);

        new G4PVPlacement(
            nullptr, gapPlacement, gapLogic, gap_name, worldLog, false, 0, false);

        new G4PVPlacement(nullptr,
                          absorberPlacement,
                          absorberLogic,
                          absorber_name,
                          worldLog,
                          false,
                          0,
                          false);

        xCenter += LayerThickness;
    }

    return worldPlaced;
}
