//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SegmentedSimpleCms.cc
//---------------------------------------------------------------------------//
#include "SegmentedSimpleCms.hh"

#include <cstdlib>
#include <G4Box.hh>
#include <G4Colour.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4PVDivision.hh>
#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4SDManager.hh>
#include <G4VisAttributes.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with geometry type enum and number of segments.
 */
SegmentedSimpleCms::SegmentedSimpleCms(MaterialType type,
                                       SegmentDefinition segments)
    : geometry_type_(type), num_segments_(segments)
{
    if (num_segments_.num_r < 1 || num_segments_.num_theta < 1
        || num_segments_.num_z < 1)
    {
        std::cout << "Number of segments must be at least 1 in every axis"
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    materials_ = this->build_materials();
}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* SegmentedSimpleCms::Construct()
{
    return this->segmented_simple_cms();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors and (TODO) magnetic field.
 */
void SegmentedSimpleCms::ConstructSDandField()
{
    this->set_sd();

    // TODO: Add magnetic field
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Define list of materials.
 */
SegmentedSimpleCms::MaterialList SegmentedSimpleCms::build_materials()
{
    MaterialList materials;
    G4NistManager* nist = G4NistManager::Instance();

    switch (geometry_type_)
    {
        case MaterialType::simple:
            // Load materials
            materials.world = nist->FindOrBuildMaterial("G4_Galactic");
            materials.vacuum_tube = nist->FindOrBuildMaterial("G4_Galactic");
            materials.si_tracker = nist->FindOrBuildMaterial("G4_Si");
            materials.em_calorimeter = nist->FindOrBuildMaterial("G4_Pb");
            materials.had_calorimeter = nist->FindOrBuildMaterial("G4_C");
            materials.sc_solenoid = nist->FindOrBuildMaterial("G4_Ti");
            materials.muon_chambers = nist->FindOrBuildMaterial("G4_Fe");

            // Update names
            materials.world->SetName("vacuum");
            materials.vacuum_tube->SetName("vacuum");
            materials.si_tracker->SetName("Si");
            materials.em_calorimeter->SetName("Pb");
            materials.had_calorimeter->SetName("C");
            materials.sc_solenoid->SetName("Ti");
            materials.muon_chambers->SetName("Fe");
            break;

        case MaterialType::composite:
            // Load materials
            materials.world = nist->FindOrBuildMaterial("G4_Galactic");
            materials.vacuum_tube = nist->FindOrBuildMaterial("G4_Galactic");
            materials.si_tracker
                = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
            materials.em_calorimeter
                = nist->FindOrBuildMaterial("G4_LEAD_OXIDE");
            materials.had_calorimeter = nist->FindOrBuildMaterial("G4_C");
            materials.sc_solenoid = nist->FindOrBuildMaterial("G4_Ti");
            materials.muon_chambers = nist->FindOrBuildMaterial("G4_Fe");

            // Update names
            materials.world->SetName("vacuum");
            materials.vacuum_tube->SetName("vacuum");
            materials.si_tracker->SetName("SiO2");
            materials.em_calorimeter->SetName("Pb3O4");
            materials.had_calorimeter->SetName("C");
            materials.sc_solenoid->SetName("Ti");
            materials.muon_chambers->SetName("Fe");
            break;
        default:
            break;
    }

    return materials;
}

//---------------------------------------------------------------------------//
/*!
 * Programmatic geometry definition: Segmented simple CMS.
 *
 * This is a set of single-material concentric cylinders that can be split
 * multiple times in each direction to generate a large numbers of volumes.
 *
 * E.g.: If the silicon tracker is segmented in 2 in the radial axis, it will
 * be split into si_tracker_1 (with r = [30, 77.5] cm) and si_tracker_2 (with
 * r = [77.5, 125] cm).
 *
 * The final size of each material cylinder is still identical to the
 * \c SimpleCms class.
 */
G4VPhysicalVolume* SegmentedSimpleCms::segmented_simple_cms()
{
    // World volume
    G4Box* world_def = new G4Box(
        "world_def", world_size_ / 2, world_size_ / 2, world_size_);

    auto const world_lv
        = new G4LogicalVolume(world_def, materials_.world, "world");

    auto const world_pv = new G4PVPlacement(0,  // Rotation matrix
                                            G4ThreeVector(),  // Position
                                            world_lv,  // Current LV
                                            "world",  // Name
                                            nullptr,  // Mother LV
                                            false,  // Bool operation
                                            0,  // Copy number
                                            false);  // Overlap check

    // Vacuum tube
    G4Tubs* vacuum_tube_def = new G4Tubs("vacuum_tube_def",
                                         0,  // Inner radius
                                         radius_.vacuum_tube,  // Outer radius
                                         half_length_,  // Half-length z
                                         0 * deg,  // Start angle
                                         360 * deg);  // Spanning angle

    auto const vacuum_tube_lv = new G4LogicalVolume(
        vacuum_tube_def, materials_.vacuum_tube, "vacuum_tube");

    new G4PVPlacement(0,
                      G4ThreeVector(),  // Spans -z/2 to +z/2
                      vacuum_tube_lv,
                      "vacuum_tube_pv",
                      world_lv,
                      false,
                      0,
                      false);

    if (!flat_segmentation)
    {
        // Use replicas and divisions to generate a hierarchical geometry

        // Si tracker
        G4Tubs* si_tracker_def = new G4Tubs("si_tracker_def",
                                            radius_.vacuum_tube,
                                            radius_.si_tracker,
                                            half_length_,
                                            0 * deg,
                                            360 * deg);

        auto const si_tracker_lv = new G4LogicalVolume(
            si_tracker_def, materials_.world, "si_tracker");

        auto si_tracker_pv = new G4PVPlacement(0,
                                               G4ThreeVector(),
                                               si_tracker_lv,
                                               "si_tracker_pv",
                                               world_lv,
                                               false,
                                               0,
                                               false);
        // EM calorimeter
        G4Tubs* em_calorimeter_def = new G4Tubs("em_calorimeter_def",
                                                radius_.si_tracker,
                                                radius_.em_calo,
                                                half_length_,
                                                0 * deg,
                                                360 * deg);

        auto const em_calorimeter_lv = new G4LogicalVolume(
            em_calorimeter_def, materials_.world, "em_calorimeter");

        new G4PVPlacement(0,
                          G4ThreeVector(),
                          em_calorimeter_lv,
                          "em_calorimeter_pv",
                          world_lv,
                          false,
                          0,
                          false);

        // Hadron calorimeter
        G4Tubs* had_calorimeter_def = new G4Tubs("had_calorimeter_def",
                                                 radius_.em_calo,
                                                 radius_.had_calo,
                                                 half_length_,
                                                 0 * deg,
                                                 360 * deg);

        auto const had_calorimeter_lv = new G4LogicalVolume(
            had_calorimeter_def, materials_.world, "had_calorimeter");

        new G4PVPlacement(0,
                          G4ThreeVector(),
                          had_calorimeter_lv,
                          "had_calorimeter_pv",
                          world_lv,
                          false,
                          0,
                          false);

        // Superconducting solenoid
        G4Tubs* sc_solenoid_def = new G4Tubs("sc_solenoid_def",
                                             radius_.had_calo,
                                             radius_.sc_solenoid,
                                             half_length_,
                                             0 * deg,
                                             360 * deg);

        auto const sc_solenoid_lv = new G4LogicalVolume(
            sc_solenoid_def, materials_.world, "sc_solenoid");

        new G4PVPlacement(0,
                          G4ThreeVector(),
                          sc_solenoid_lv,
                          "sc_solenoid_pv",
                          world_lv,
                          false,
                          0,
                          false);

        // Muon chambers
        G4Tubs* muon_chambers_def = new G4Tubs("muon_chambers_def",
                                               radius_.sc_solenoid,
                                               radius_.muon_chambers,
                                               half_length_,
                                               0 * deg,
                                               360 * deg);

        auto const muon_chambers_lv = new G4LogicalVolume(
            muon_chambers_def, materials_.world, "muon_chambers");

        new G4PVPlacement(0,
                          G4ThreeVector(),
                          muon_chambers_lv,
                          "muon_chambers_pv",
                          world_lv,
                          false,
                          0,
                          false);

        // Add segmented elements
        this->create_segments("si_tracker",
                              radius_.vacuum_tube,
                              radius_.si_tracker,
                              si_tracker_def,
                              si_tracker_lv,
                              materials_.si_tracker);

        this->create_segments("em_calorimeter",
                              radius_.si_tracker,
                              radius_.em_calo,
                              em_calorimeter_def,
                              em_calorimeter_lv,
                              materials_.em_calorimeter);

        this->create_segments("had_calorimeter",
                              radius_.em_calo,
                              radius_.had_calo,
                              had_calorimeter_def,
                              had_calorimeter_lv,
                              materials_.had_calorimeter);

        this->create_segments("sc_solenoid",
                              radius_.had_calo,
                              radius_.sc_solenoid,
                              sc_solenoid_def,
                              sc_solenoid_lv,
                              materials_.sc_solenoid);

        this->create_segments("muon_chambers",
                              radius_.sc_solenoid,
                              radius_.muon_chambers,
                              muon_chambers_def,
                              muon_chambers_lv,
                              materials_.muon_chambers);
    }

    else
    {
        // Manually place every segment directly into the world volume
        this->flat_segmented_cylinder("si_tracker",
                                      radius_.vacuum_tube,
                                      radius_.si_tracker,
                                      materials_.si_tracker,
                                      world_pv);

        this->flat_segmented_cylinder("em_calorimeter",
                                      radius_.si_tracker,
                                      radius_.em_calo,
                                      materials_.em_calorimeter,
                                      world_pv);

        this->flat_segmented_cylinder("had_calorimeter",
                                      radius_.em_calo,
                                      radius_.had_calo,
                                      materials_.had_calorimeter,
                                      world_pv);

        this->flat_segmented_cylinder("sc_solenoid",
                                      radius_.had_calo,
                                      radius_.sc_solenoid,
                                      materials_.sc_solenoid,
                                      world_pv);

        this->flat_segmented_cylinder("muon_chambers",
                                      radius_.sc_solenoid,
                                      radius_.muon_chambers,
                                      materials_.muon_chambers,
                                      world_pv);
    }

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * TODO
 */
void SegmentedSimpleCms::set_sd()
{
    // TODO
}

//---------------------------------------------------------------------------//
/*!
 * Generate segments in r, theta, and z for a given cylinder using replicas and
 * divisions.
 *
 * \note Not compatible with VecGeom.
 */
void SegmentedSimpleCms::create_segments(std::string name,
                                         double inner_r,
                                         double outer_r,
                                         G4Tubs* full_culinder_def,
                                         G4LogicalVolume* full_cylinder_lv,
                                         G4Material* cyl_material)
{
    std::string name_segment = name + "_segment";
    std::string name_r = name_segment + "_r";
    std::string name_z = name_segment + "_z";
    std::string name_theta = name_segment + "_theta";

    // Theta
    double const segment_theta = 2 * CLHEP::pi / num_segments_.num_theta;

    std::string name_theta_def = name_theta + "_def";
    G4Tubs* segment_theta_def = new G4Tubs(
        name_theta_def, inner_r, outer_r, half_length_, 0, segment_theta);

    std::string name_theta_lv = name_theta + "";
    auto const segment_theta_lv = new G4LogicalVolume(
        full_culinder_def, materials_.world, name_theta_lv);

    // R
    double const segment_r = (outer_r - inner_r) / num_segments_.num_r;

    std::string name_r_def = name_r + "_def";
    G4Tubs* segment_r_def = new G4Tubs(
        name_r_def, inner_r, inner_r + segment_r, half_length_, 0, segment_theta);

    std::string name_r_lv = name_r + "";
    auto const segment_r_lv
        = new G4LogicalVolume(segment_r_def, materials_.world, name_r_lv);

    std::string name_r_pv = name_r + "_pv";
    new G4PVReplica(name_r_pv,
                    segment_r_lv,
                    segment_theta_lv,
                    EAxis::kRho,
                    num_segments_.num_r,
                    0);

    std::string name_theta_pv = name_theta + "_pv";
    new G4PVReplica(name_theta_pv,
                    segment_theta_lv,
                    full_cylinder_lv,
                    EAxis::kPhi,
                    num_segments_.num_theta,
                    segment_theta);

    // Z
    double const segment_z = 2 * half_length_ / num_segments_.num_z;

    std::string name_z_def = name_z + "_def";
    G4Tubs* si_tracker_segment_z_def = new G4Tubs(name_z_def,
                                                  inner_r,
                                                  inner_r + segment_r,
                                                  segment_z / 2,
                                                  0,
                                                  segment_theta);

    std::string name_z_lv = name_z + "";
    auto const si_tracker_segment_z_lv = new G4LogicalVolume(
        si_tracker_segment_z_def, cyl_material, name_z_lv);

    std::string name_z_pv = name_z + "_pv";
    new G4PVDivision(name_z_pv,
                     si_tracker_segment_z_lv,
                     segment_r_lv,
                     EAxis::kZAxis,
                     num_segments_.num_z,
                     0);
}

//---------------------------------------------------------------------------//
/*!
 * Helper function to generate a flat segmented cylinder: this manually
 * constructs individual G4Tubs as segments, and place them directly into the
 * world volume.
 *
 * \note Compatible with VecGeom.
 */
void SegmentedSimpleCms::flat_segmented_cylinder(std::string name,
                                                 double inner_r,
                                                 double outer_r,
                                                 G4Material* material,
                                                 G4VPhysicalVolume* world_pv)
{
    // Segment sizes
    double const segment_r = (outer_r - inner_r) / num_segments_.num_r;
    double const segment_theta = (2 * CLHEP::pi) / num_segments_.num_theta;
    double const segment_z = (2 * half_length_) / num_segments_.num_z;
    double const half_segment_z = segment_z / 2;

    // Initial z position
    double const init_z = -half_length_ + half_segment_z;

    for (int r = 0; r < num_segments_.num_r; r++)
    {
        double const r_min = inner_r + r * segment_r;
        double const r_max = r_min + segment_r;

        for (int z = 0; z < num_segments_.num_z; z++)
        {
            for (int theta = 0; theta < num_segments_.num_theta; theta++)
            {
                double const theta_min = theta * segment_theta;

                std::string segment_name = name + "_" + std::to_string(r) + "_"
                                           + std::to_string(z) + "_"
                                           + std::to_string(theta);
                std::string segment_def_str = segment_name + "_def";
                std::string segment_pv_str = segment_name + "_pv";
                auto segment_def = new G4Tubs(segment_def_str,
                                              r_min,
                                              r_max,
                                              half_segment_z,
                                              theta_min,
                                              segment_theta);

                auto segment_lv
                    = new G4LogicalVolume(segment_def, material, segment_name);

                G4ThreeVector pos;
                pos.setRhoPhiZ(0, 0, init_z + z * segment_z);

                new G4PVPlacement(
                    0, pos, segment_pv_str, segment_lv, world_pv, 0, 0, false);
            }
        }
    }
}
