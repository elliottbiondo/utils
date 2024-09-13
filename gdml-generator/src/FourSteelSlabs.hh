//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file FourSteelSlabs.hh
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
class FourSteelSlabs : public G4VUserDetectorConstruction
{
  public:
    // Construct empty
    FourSteelSlabs();

    // Construct geometry
    G4VPhysicalVolume* Construct() final;

  private:
    // Four stainless-steel slabs in a vacuum
    G4VPhysicalVolume* create_geometry();
};
