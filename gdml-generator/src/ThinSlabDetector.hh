//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file BoxDetector.hh
//! \brief Create the detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4Material.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a programmatic detector geometry.
 */
class ThinSlabDetector : public G4VUserDetectorConstruction
{
  public:
    // Construct
    ThinSlabDetector();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;
    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;

  private:
    struct SlabDefinition
    {
        G4Material* material;
        std::array<double, 3> dimension;
    };

    // Pb slab of 5 cm x 5 cm x 5 um.
    SlabDefinition lead_slab_def();
    // Carbon slab of 5 cm x 5 cm x 10 um.
    SlabDefinition carbon_slab_def();
    // Thin slab for MSC validation
    G4VPhysicalVolume* create_slab(SlabDefinition const& def);
    // Set volume as sensitive detector
    void set_sd();
};
