//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SegmentedSimpleCmsDetector.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4Material.hh>
#include <G4SystemOfUnits.hh>
#include <G4Tubs.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class SegmentedSimpleCmsDetector : public G4VUserDetectorConstruction
{
  public:
    enum class MaterialType
    {
        simple,
        composite
    };

    struct SegmentDefinition
    {
        int num_theta;
        int num_r;
        int num_z;
    };

    // Construct with geometry type and number of segments
    SegmentedSimpleCmsDetector(MaterialType type, SegmentDefinition segments);

    // Construct geometry
    G4VPhysicalVolume* Construct() final;

    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;

  private:
    // Material selection
    struct MaterialList
    {
        G4Material* world;
        G4Material* vacuum_tube;
        G4Material* si_tracker;
        G4Material* em_calorimeter;
        G4Material* had_calorimeter;
        G4Material* sc_solenoid;
        G4Material* muon_chambers;
    } materials_;

    // Size of World volume
    double const world_size_ = 20 * m;

    // Hardcoded cylinder sizes
    struct CylinderRadius
    {
        double vacuum_tube{30 * cm};
        double si_tracker{125 * cm};
        double em_calo{175 * cm};
        double had_calo{275 * cm};
        double sc_solenoid{375 * cm};
        double muon_chambers{700 * cm};
    } const radius_;

    // Cylinder half length
    double const half_length_{7 * m};

    // Select simple/composite geometry
    MaterialType geometry_type_;

    // Number of segments
    SegmentDefinition num_segments_;

    // Use flat geometry construction
    bool const flat_segmentation{true};

  private:
    // Segmented simple CMS using replicas and divisions
    G4VPhysicalVolume* segmented_simple_cms();

    // Set sensitive detectors
    void set_sd();

    // Build material list based on MaterialType
    MaterialList build_materials();

    // Construct segments for each main cylinder using replicas and divisions
    void create_segments(std::string name,
                         double inner_r,
                         double outer_r,
                         G4Tubs* full_culinder_def,
                         G4LogicalVolume* full_cylinder_lv,
                         G4Material* cyl_material);

    // Segmented simple CMS with manually placed volumes
    G4VPhysicalVolume* flat_segmented_simple_cms();

    // Construct a segmented cylinder by manually placing smaller ones
    void flat_segmented_cylinder(std::string name,
                                 double inner_r,
                                 double outer_r,
                                 G4Material* material,
                                 G4VPhysicalVolume* world_pv);
};
