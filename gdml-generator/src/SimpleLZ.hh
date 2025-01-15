//----------------------------------*-C++-*----------------------------------//
// Copyright 2024-2025 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SimpleLZ.hh
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
class SimpleLZ : public G4VUserDetectorConstruction
{
  public:
    // Construct
    SimpleLZ();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;

    // Set up sensitive detectors and magnetic field
    void ConstructSDandField() final;
};
