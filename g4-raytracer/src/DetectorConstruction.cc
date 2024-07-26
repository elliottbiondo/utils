//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
#include "DetectorConstruction.hh"

#include <G4GDMLParser.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct with GDML filename.
 */
DetectorConstruction::DetectorConstruction(std::string input_gdml)
    : G4VUserDetectorConstruction()
{
    G4GDMLParser parser;
    parser.Read(input_gdml, false);
    phys_vol_world_.reset(parser.GetWorldVolume());
}

//---------------------------------------------------------------------------//
/*!
 * Construct geometry from GDML.
 */
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    return phys_vol_world_.release();
}
