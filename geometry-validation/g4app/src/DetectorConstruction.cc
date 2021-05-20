//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.cc
//---------------------------------------------------------------------------//
#include "DetectorConstruction.hh"

#include <fstream>

#include <G4NistManager.hh>
#include <G4GeometryManager.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4SolidStore.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4SystemOfUnits.hh>
#include <G4PhysicalConstants.hh>
#include <G4GDMLParser.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct geometry.
 *
 * The programmatic geometry is direct, whilst the GDML geometry is created by
 * exporting the programmatic one to a gdml file and parsing it to load the
 * geometry into memory.
 *
 * Three testing options are added:
 * 0: single-material cms mockup
 * 1: Geant4/examples/basic/B1
 * 2: Celeritas' four-steel-slabs
 * 3: Geant4/examples/advanced/amsEcal
 */
DetectorConstruction::DetectorConstruction(Geometry selection, bool from_gdml)
    : selected_geometry_(selection)
{
    this->set_gdml_filename();

    if (!from_gdml)
    {
        this->set_phys_volume();
        std::ifstream gdml_input(this->get_gdml_filename());

        if (!gdml_input.good())
        {
            this->export_gdml();
        }
    }

    else
    {
        // Check if gdml already exists
        std::ifstream gdml_input(this->get_gdml_filename());

        if (!gdml_input.good())
        {
            // Gdml was not exported by a previous execution; create it
            this->set_phys_volume();
            this->export_gdml();
            std::cout << "\nWARNING: Geant4 singletons will not work properly "
                         "after creating a programmatic geometry, exporting "
                         "it, and parsing the gdml file as the expected "
                         "geometry. Rerun the code for a correct execution.\n"
                      << std::endl;
        }

        G4GDMLParser   gdml_parser;
        constexpr bool validate_gdml_schema = false;
        gdml_parser.Read(this->get_gdml_filename(), validate_gdml_schema);
        world_phys_vol_.reset(gdml_parser.GetWorldVolume());
    }
}

//---------------------------------------------------------------------------//
/*!
 * Get constructed world physical volume.
 */
const G4VPhysicalVolume* DetectorConstruction::get_world_volume() const
{
    return world_phys_vol_.get();
}

//---------------------------------------------------------------------------//
/*!
 * Construct geometry. Called by Geant4 Run Manager.
 */
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    return world_phys_vol_.release();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

void DetectorConstruction::set_phys_volume()
{
    // Construct programmatic geometry
    switch (selected_geometry_)
    {
        case Geometry::simple_cms:
            world_phys_vol_.reset(this->create_simple_cms_geometry());
            break;
        case Geometry::example_b1:
            world_phys_vol_.reset(this->create_b1_geometry());
            break;
        case Geometry::four_steel_slabs:
            world_phys_vol_.reset(this->create_slabs_geometry());
            break;
        case Geometry::ams_ecal:
            world_phys_vol_.reset(this->create_ams_ecal_geometry());
            break;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Set up gdml filename for export.
 */
void DetectorConstruction::set_gdml_filename()
{
    switch (selected_geometry_)
    {
        case Geometry::simple_cms:
            gdml_filename_ = "simple-cms.gdml";
            break;
        case Geometry::example_b1:
            gdml_filename_ = "example-b1.gdml";
            break;
        case Geometry::four_steel_slabs:
            gdml_filename_ = "four-steel-slabs.gdml";
            break;
        case Geometry::ams_ecal:
            gdml_filename_ = "ams-ecal.gdml";
            break;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Export GDML file representing the programmatic geometry.
 */
void DetectorConstruction::export_gdml()
{
    G4GDMLParser parser;
    parser.Write(this->get_gdml_filename(), world_phys_vol_.get());
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Single material CMS mock up.
 *
 * This is set of single-element concentric cylinders that act as a
 * spherical cow in a vacuum version of CMS.
 */
G4VPhysicalVolume* DetectorConstruction::create_simple_cms_geometry()
{
    // Create all used materials
    G4NistManager* nist   = G4NistManager::Instance();
    G4Material*    vacuum = nist->FindOrBuildMaterial("G4_Galactic"); // H
                                                                      // only
    G4Material* Si = nist->FindOrBuildMaterial("G4_Si");
    G4Material* Pb = nist->FindOrBuildMaterial("G4_Pb");
    G4Material* C  = nist->FindOrBuildMaterial("G4_C");
    G4Material* Ti = nist->FindOrBuildMaterial("G4_Ti");
    G4Material* Fe = nist->FindOrBuildMaterial("G4_Fe");

    // Size of World volume
    const double world_size = 20 * m;
    // Half length of all concentric cylinders (z-axis)
    const double half_length = 7 * m;
    // Small distance between cylinder edges to avoid volume overlap
    const double delta = 1e-10;

    // Create solids
    G4Box* world_box
        = new G4Box("world_box", world_size / 2, world_size / 2, world_size);

    G4Tubs* si_tracker = new G4Tubs("silicon_tracker",
                                    30 * cm,          // Inner radius
                                    125 * cm - delta, // Outer radius
                                    half_length,      // Half-length z
                                    0 * deg,          // Start angle
                                    360 * deg);       // Spanning angle

    G4Tubs* em_calorimeter = new G4Tubs("crystal_em_calorimeter",
                                        125 * cm,
                                        175 * cm - delta,
                                        half_length,
                                        0 * deg,
                                        360 * deg);

    G4Tubs* had_calorimeter = new G4Tubs("hadron_calorimeter",
                                         175 * cm,
                                         275 * cm - delta,
                                         half_length,
                                         0 * deg,
                                         360 * deg);

    G4Tubs* sc_solenoid = new G4Tubs("superconducting_solenoid",
                                     275 * cm,
                                     375 * cm - delta,
                                     half_length,
                                     0 * deg,
                                     360 * deg);

    G4Tubs* iron_muon_chambers = new G4Tubs("iron_muon_chambers",
                                            375 * cm,
                                            700 * cm,
                                            half_length,
                                            0 * deg,
                                            360 * deg);

    // Create logical volumes
    const auto world_lv = new G4LogicalVolume(world_box, vacuum, "world_lv");
    const auto si_tracker_lv
        = new G4LogicalVolume(si_tracker, Si, "si_tracker_lv");
    const auto em_calorimeter_lv
        = new G4LogicalVolume(em_calorimeter, Pb, "em_calorimeter_lv");
    const auto had_calorimeter_lv
        = new G4LogicalVolume(had_calorimeter, C, "had_calorimeter_lv");
    const auto sc_solenoid_lv
        = new G4LogicalVolume(sc_solenoid, Ti, "sc_solenoid_lv");
    const auto iron_muon_chambers_lv
        = new G4LogicalVolume(iron_muon_chambers, Fe, "iron_muon_chambers_lv");

    // Create physical volumes
    const auto world_pv = new G4PVPlacement(0,               // Rotation matrix
                                            G4ThreeVector(), // Position
                                            world_lv,        // Current LV
                                            "world_pv",      // Name
                                            nullptr,         // Mother LV
                                            false,           // Bool operation
                                            0,               // Copy number
                                            true);           // Overlap check

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      si_tracker_lv,
                      "si_tracker_pv",
                      world_lv,
                      false,
                      0,
                      true);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      em_calorimeter_lv,
                      "em_calorimeter_pv",
                      world_lv,
                      false,
                      0,
                      true);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      had_calorimeter_lv,
                      "had_calorimeter_pv",
                      world_lv,
                      false,
                      0,
                      true);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      sc_solenoid_lv,
                      "sc_solenoid_pv",
                      world_lv,
                      false,
                      0,
                      true);

    new G4PVPlacement(0,
                      G4ThreeVector(),
                      iron_muon_chambers_lv,
                      "iron_muon_chambers_"
                      "pv",
                      world_lv,
                      false,
                      0,
                      true);

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Geant4/examples/basic/B1.
 *
 * (Direct copy from the original Geant4 example, thus the poor variable
 * naming and comments.)
 */
G4VPhysicalVolume* DetectorConstruction::create_b1_geometry()
{
    // Get nist material manager
    G4NistManager* nist = G4NistManager::Instance();

    // Envelope parameters
    double      env_sizeXY = 20 * cm, env_sizeZ = 30 * cm;
    G4Material* env_mat = nist->FindOrBuildMaterial("G4_WATER");

    bool checkOverlaps = true;

    // World
    double      world_sizeXY = 1.2 * env_sizeXY;
    double      world_sizeZ  = 1.2 * env_sizeZ;
    G4Material* world_mat    = nist->FindOrBuildMaterial("G4_AIR");

    G4Box* solidWorld = new G4Box("World", // its name
                                  0.5 * world_sizeXY,
                                  0.5 * world_sizeXY,
                                  0.5 * world_sizeZ); // its size

    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, // its
                                                                  // solid
                                                      world_mat,  // its
                                                                  // material
                                                      "World");   // its name

    G4VPhysicalVolume* physWorld
        = new G4PVPlacement(0,               // no rotation
                            G4ThreeVector(), // at (0,0,0)
                            logicWorld,      // its logical volume
                            "World",         // its name
                            0,               // its mother  volume
                            false,           // no boolean operation
                            0,               // copy number
                            checkOverlaps);  // overlaps checking

    //
    // Envelope
    //
    G4Box* solidEnv = new G4Box("Envelope", // its name
                                0.5 * env_sizeXY,
                                0.5 * env_sizeXY,
                                0.5 * env_sizeZ); // its size

    G4LogicalVolume* logicEnv = new G4LogicalVolume(solidEnv,    // its solid
                                                    env_mat,     // its
                                                                 // material
                                                    "Envelope"); // its
                                                                 // name

    new G4PVPlacement(0,               // no rotation
                      G4ThreeVector(), // at (0,0,0)
                      logicEnv,        // its logical volume
                      "Envelope",      // its name
                      logicWorld,      // its mother  volume
                      false,           // no boolean operation
                      0,               // copy number
                      checkOverlaps);  // overlaps checking

    //
    // Shape 1
    //
    G4Material*   shape1_mat = nist->FindOrBuildMaterial("G4_A-150_TISSUE");
    G4ThreeVector pos1       = G4ThreeVector(0, 2 * cm, -7 * cm);

    // Conical section shape
    G4double shape1_rmina = 0. * cm, shape1_rmaxa = 2. * cm;
    G4double shape1_rminb = 0. * cm, shape1_rmaxb = 4. * cm;
    G4double shape1_hz     = 3. * cm;
    G4double shape1_phimin = 0. * deg, shape1_phimax = 360. * deg;
    G4Cons*  solidShape1 = new G4Cons("Shape1",
                                     shape1_rmina,
                                     shape1_rmaxa,
                                     shape1_rminb,
                                     shape1_rmaxb,
                                     shape1_hz,
                                     shape1_phimin,
                                     shape1_phimax);

    G4LogicalVolume* logicShape1 = new G4LogicalVolume(solidShape1, // Solid
                                                       shape1_mat,  // Material
                                                       "Shape1");   // Name

    new G4PVPlacement(0,              // no rotation
                      pos1,           // at position
                      logicShape1,    // its logical volume
                      "Shape1",       // its name
                      logicEnv,       // its mother  volume
                      false,          // no boolean operation
                      0,              // copy number
                      checkOverlaps); // overlaps checking

    // Shape 2
    G4Material* shape2_mat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
    G4ThreeVector pos2     = G4ThreeVector(0, -1 * cm, 7 * cm);

    // Trapezoid shape
    G4double shape2_dxa = 12 * cm, shape2_dxb = 12 * cm;
    G4double shape2_dya = 10 * cm, shape2_dyb = 16 * cm;
    G4double shape2_dz   = 6 * cm;
    G4Trd*   solidShape2 = new G4Trd("Shape2", // its name
                                   0.5 * shape2_dxa,
                                   0.5 * shape2_dxb,
                                   0.5 * shape2_dya,
                                   0.5 * shape2_dyb,
                                   0.5 * shape2_dz); // its size

    G4LogicalVolume* logicShape2 = new G4LogicalVolume(solidShape2, // its
                                                                    // solid
                                                       shape2_mat,  // its
                                                                    // material
                                                       "Shape2");   // its name

    new G4PVPlacement(0,              // no rotation
                      pos2,           // at position
                      logicShape2,    // its logical volume
                      "Shape2",       // its name
                      logicEnv,       // its mother  volume
                      false,          // no boolean operation
                      0,              // copy number
                      checkOverlaps); // overlaps checking

    // Return the physical World
    return physWorld;
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Celeritas' four-steel-slabs.gdml
 */
G4VPhysicalVolume* DetectorConstruction::create_slabs_geometry()
{
    // Geometry materials
    const auto nist_manager = G4NistManager::Instance();
    const auto world_material
        = nist_manager->FindOrBuildMaterial("G4_Galactic");
    const auto slab_material
        = nist_manager->FindOrBuildMaterial("G4_STAINLESS-STEEL");

    // World definition
    const double world_xy = 1000. * cm;
    const double world_z  = 1000. * cm;

    const auto world_solid
        = new G4Box("World", world_xy / 2, world_xy / 2, world_z / 2);

    const auto world_log_vol
        = new G4LogicalVolume(world_solid, world_material, "World");

    const auto world_phys_vol = new G4PVPlacement(0, // No rotation
                                                  G4ThreeVector(), // (0,0,0)
                                                  world_log_vol,   // Log vol
                                                  "World",         // Name
                                                  0,     // Mother volume
                                                  false, // No bool operation
                                                  0,     // Copy number
                                                  true); // Check overlaps

    // Slabs definition
    const auto slabs_xy = 0.01 * world_xy;
    const auto slabs_z  = 0.2 * slabs_xy;

    // Slab 0
    const auto slab_solid = new G4Box("box", slabs_xy, slabs_xy, slabs_z);

    const auto slab_log_vol
        = new G4LogicalVolume(slab_solid, slab_material, "box");

    new G4PVPlacement(
        0, G4ThreeVector(), slab_log_vol, "box", world_log_vol, false, 0, true);

    // Slab 1
    const auto slab_replica_solid
        = new G4Box("boxReplica", slabs_xy, slabs_xy, slabs_z);

    const auto slab_replica_log_vol
        = new G4LogicalVolume(slab_replica_solid, slab_material, "boxReplica");

    new G4PVPlacement(0,
                      G4ThreeVector(0, 0, 3 * slabs_z),
                      slab_replica_log_vol,
                      "box",
                      world_log_vol,
                      false,
                      0,
                      true);

    // Slab 2
    const auto slab_replica_2_solid
        = new G4Box("boxReplica2", slabs_xy, slabs_xy, slabs_z);

    const auto slab_replica_2_log_vol = new G4LogicalVolume(
        slab_replica_2_solid, slab_material, "boxReplica");

    new G4PVPlacement(0,
                      G4ThreeVector(0, 0, 6 * slabs_z),
                      slab_replica_2_log_vol,
                      "box",
                      world_log_vol,
                      false,
                      0,
                      true);

    // Slab 3
    const auto slab_replica_3_solid
        = new G4Box("boxReplica3", slabs_xy, slabs_xy, slabs_z);

    const auto slab_replica_3_log_vol = new G4LogicalVolume(
        slab_replica_3_solid, slab_material, "boxReplica");

    new G4PVPlacement(0,
                      G4ThreeVector(0, 0, 9 * slabs_z),
                      slab_replica_3_log_vol,
                      "box",
                      world_log_vol,
                      false,
                      0,
                      true);

    // Visualization attributes
    world_log_vol->SetVisAttributes(G4VisAttributes::GetInvisible());
    const auto slab_vis_att = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0));
    slab_vis_att->SetVisibility(true);
    slab_log_vol->SetVisAttributes(slab_vis_att);
    slab_replica_log_vol->SetVisAttributes(slab_vis_att);
    slab_replica_2_log_vol->SetVisAttributes(slab_vis_att);
    slab_replica_3_log_vol->SetVisAttributes(slab_vis_att);

    return world_phys_vol;
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Geant4/examples/advanced/amsEcal.
 *
 * (Direct copy from the original Geant4 example, thus the poor variable
 * naming and comments.)
 */
G4VPhysicalVolume* DetectorConstruction::create_ams_ecal_geometry()
{
    // define Elements
    //
    G4Element* H = new G4Element("Hydrogen", "H", 1, 1.01 * g / mole);
    G4Element* C = new G4Element("Carbon", "C", 6, 12.01 * g / mole);
    G4Element* N = new G4Element("Nitrogen", "N", 7, 14.01 * g / mole);
    G4Element* O = new G4Element("Oxygen", "O", 8, 16.00 * g / mole);

    G4int    natoms, ncomponents;
    G4double density, massfraction;

    // Lead
    //
    G4Material* Pb = new G4Material(
        "Lead", 82., 207.20 * g / mole, density = 0.98 * 11.20 * g / cm3);

    // Scintillator
    //
    G4Material* Sci = new G4Material(
        "Scintillator", density = 1.032 * g / cm3, ncomponents = 2);
    Sci->AddElement(C, natoms = 8);
    Sci->AddElement(H, natoms = 8);

    Sci->GetIonisation()->SetBirksConstant(0.126 * mm / MeV);

    // Air
    //
    G4Material* Air
        = new G4Material("Air", density = 1.290 * mg / cm3, ncomponents = 2);
    Air->AddElement(N, massfraction = 70 * perCent);
    Air->AddElement(O, massfraction = 30. * perCent);

    // example of vacuum
    //
    density              = universe_mean_density; // from PhysicalConstants.h
    G4double    pressure = 3.e-18 * pascal;
    G4double    temperature = 2.73 * kelvin;
    G4Material* Vacuum      = new G4Material("Galactic",
                                        1.,
                                        1.008 * g / mole,
                                        density,
                                        kStateGas,
                                        temperature,
                                        pressure);

    // attribute materials
    //
    const auto defaultMat     = Vacuum;
    const auto fiberMat       = Sci;
    const auto absorberMat    = Pb;
    const auto moduleMat      = defaultMat;
    const auto calorimeterMat = defaultMat;
    const auto worldMat       = defaultMat;

    // default parameter values of calorimeter
    //
    G4double fiberDiameter       = 1.13 * mm; // 1.08*mm
    G4int    nbOfFibers          = 490;       // 490
    G4double distanceInterFibers = 1.35 * mm; // 1.35*mm
    G4double layerThickness      = 1.73 * mm; // 1.68*mm
    G4double milledLayer         = 1.00 * mm; // 1.40*mm ?
    G4int    nbOfLayers          = 10;        // 10
    G4int    nbOfModules         = 9;         // 9
    G4double fiberLength         = (nbOfFibers + 0.5)
                           * distanceInterFibers; // 662.175*mm

    // fibers
    //
    G4Tubs* svol_fiber = new G4Tubs("fiber", // name
                                    0 * mm,
                                    0.5 * fiberDiameter, // r1, r2
                                    0.5 * fiberLength,   // half-length
                                    0.,
                                    twopi); // theta1, theta2

    const auto lvol_fiber = new G4LogicalVolume(svol_fiber, // solid
                                                fiberMat,   // material
                                                "fiber");   // name

    // layer
    //
    G4double sizeX = layerThickness;
    G4double sizeY = distanceInterFibers * nbOfFibers;
    G4double sizeZ = fiberLength;

    G4Box* svol_layer = new G4Box("layer", // name
                                  0.5 * sizeX,
                                  0.5 * sizeY,
                                  0.5 * sizeZ); // size

    const auto lvol_layer = new G4LogicalVolume(svol_layer,  // solid
                                                absorberMat, // material
                                                "layer");    // name

    // put fibers within layer
    //
    G4double Xcenter = 0.;
    G4double Ycenter = -0.5 * (sizeY + distanceInterFibers);

    for (G4int k = 0; k < nbOfFibers; k++)
    {
        Ycenter += distanceInterFibers;
        new G4PVPlacement(0,                                   // no rotation
                          G4ThreeVector(Xcenter, Ycenter, 0.), // position
                          lvol_fiber, // logical volume
                          "fiber",    // name
                          lvol_layer, // mother
                          false,      // no boulean operat
                          k + 1);     // copy number
    }

    // modules
    //
    G4double moduleThickness = layerThickness * nbOfLayers + milledLayer;
    sizeX                    = moduleThickness;
    sizeY                    = fiberLength;
    sizeZ                    = fiberLength;

    G4Box* svol_module = new G4Box("module", // name
                                   0.5 * sizeX,
                                   0.5 * sizeY,
                                   0.5 * sizeZ); // size

    const auto lvol_module = new G4LogicalVolume(svol_module, // solid
                                                 absorberMat, // material
                                                 "module");   // name

    // put layers within module
    //
    Xcenter = -0.5 * (nbOfLayers + 1) * layerThickness;
    Ycenter = 0.25 * distanceInterFibers;

    for (G4int k = 0; k < nbOfLayers; k++)
    {
        Xcenter += layerThickness;
        Ycenter = -Ycenter;
        new G4PVPlacement(0,                                   // no rotation
                          G4ThreeVector(Xcenter, Ycenter, 0.), // position
                          lvol_layer,  // logical volume
                          "layer",     // name
                          lvol_module, // mother
                          false,       // no boulean operat
                          k + 1);      // copy number
    }

    // calorimeter
    //
    G4double calorThickness = moduleThickness * nbOfModules;
    sizeX                   = calorThickness;
    sizeY                   = fiberLength;
    sizeZ                   = fiberLength;

    G4Box* svol_calorimeter = new G4Box("calorimeter", // name
                                        0.5 * sizeX,
                                        0.5 * sizeY,
                                        0.5 * sizeZ); // size

    const auto lvol_calorimeter
        = new G4LogicalVolume(svol_calorimeter, // solid
                              calorimeterMat,   // material
                              "calorimeter");   // name

    // put modules inside calorimeter
    //
    Xcenter = -0.5 * (calorThickness + moduleThickness);

    for (G4int k = 0; k < nbOfModules; k++)
    {
        Xcenter += moduleThickness;
        G4RotationMatrix rotm; // rotation matrix to place modules
        if ((k + 1) % 2 == 0)
            rotm.rotateX(90 * deg);
        G4Transform3D transform(rotm, G4ThreeVector(Xcenter, 0., 0.));
        new G4PVPlacement(transform,        // rotation+position
                          lvol_module,      // logical volume
                          "module",         // name
                          lvol_calorimeter, // mother
                          false,            // no boulean operat
                          k + 1);           // copy number
    }

    // world
    //
    sizeX = 1.2 * calorThickness;
    sizeY = 1.2 * fiberLength;
    sizeZ = 1.2 * fiberLength;

    G4double worldSizeX = sizeX;

    G4Box* svol_world = new G4Box("world", // name
                                  0.5 * sizeX,
                                  0.5 * sizeY,
                                  0.5 * sizeZ); // size

    const auto lvol_world = new G4LogicalVolume(svol_world, // solid
                                                worldMat,   // material
                                                "world");   // name

    const auto pvol_world = new G4PVPlacement(0,               // no rotation
                                              G4ThreeVector(), // at
                                                               // (0,0,0)
                                              lvol_world, // logical volume
                                              "world",    // name
                                              0,          // mother  volume
                                              false,      // no boolean
                                                          // operation
                                              0);         // copy number

    // put calorimeter in world
    //
    new G4PVPlacement(0,                // no rotation
                      G4ThreeVector(),  // at (0,0,0)
                      lvol_calorimeter, // logical volume
                      "calorimeter",    // name
                      lvol_world,       // mother  volume
                      false,            // no boolean operation
                      0);               // copy number

    // always return the physical World
    //
    return pvol_world;
}
