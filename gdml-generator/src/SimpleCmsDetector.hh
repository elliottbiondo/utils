//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SimpleCmsDetector.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4Material.hh>
#include <G4SystemOfUnits.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class SimpleCmsDetector : public G4VUserDetectorConstruction
{
  public:
    enum class MaterialType
    {
        simple,
        composite
    };

    // Construct with geometry type
    SimpleCmsDetector(MaterialType type);

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
    };

    // Volume gap definition
    struct VolumeGap
    {
        double overlap{0};
        double millimeter{1 * mm};
        double tolerance{1e-9 * mm};
    } const volume_gaps_;

    // Select simple/composite geometry
    MaterialType geometry_type_;

  private:
    // Simple CMS mock up
    G4VPhysicalVolume* simple_cms();
    // Set simple cms volumes as sensitive detectors
    void set_sd();
    // Build material list based on MaterialType
    MaterialList build_materials();
};
