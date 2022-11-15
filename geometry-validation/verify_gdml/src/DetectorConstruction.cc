//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.cc
//---------------------------------------------------------------------------//
#include "DetectorConstruction.hh"

#include <G4GDMLParser.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct geometry with GDML filename
 */
DetectorConstruction::DetectorConstruction(std::string gdml_filename)

{
    G4GDMLParser gdml_parser;
    gdml_parser.Read(gdml_filename, false);
    world_phys_vol_.reset(gdml_parser.GetWorldVolume());
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
/*!
 * Get constructed world physical volume.
 */
const G4VPhysicalVolume* DetectorConstruction::get_world_volume() const
{
    return world_phys_vol_.get();
}
