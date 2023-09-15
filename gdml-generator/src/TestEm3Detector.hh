//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TestEm3Detector.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <G4VUserDetectorConstruction.hh>

class G4VPhysicalVolume;
class G4Material;

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 *
 * Programmatic geometry definition taken from AdePT's repository.
 * See github.com/apt-sim/AdePT/tree/master/examples/TestEm3
 */
class TestEm3Detector : public G4VUserDetectorConstruction
{
  public:
    enum class MaterialType
    {
        simple,
        composite,
    };

    enum class GeometryType
    {
        hierarchical,
        flat
    };

    // Construct empty
    TestEm3Detector(MaterialType material_type, GeometryType geometry_type);

    // Construct geometry
    G4VPhysicalVolume* Construct() final;
    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;

  private:
    // Material selection
    struct MaterialList
    {
        G4Material* world;
        G4Material* gap;
        G4Material* absorber;
    };

  private:
    // Set up materials based on Type enum
    MaterialList load_materials();
    // Define programmatic geometry: AdePT's examples/TestEm3
    G4VPhysicalVolume* create_testem3();
    // Flatenned TestEm3 geometry for ORANGE
    G4VPhysicalVolume* create_testem3_flat();
    // Set TestEm3 volumes as sensitive detectors
    void set_sd();

  private:
    // Assigned by create_testem3() and released ownership at Construct()
    std::unique_ptr<G4VPhysicalVolume> phys_vol_world_;
    GeometryType                       geometry_type_;
    MaterialType                       material_type_;
};
