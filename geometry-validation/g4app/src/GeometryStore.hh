//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file GeometryStore.hh
//! \brief Store detector geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include "Data.hh"

//---------------------------------------------------------------------------//
/*!
 * Map detector geometry for comparison purposes.
 */
class GeometryStore
{
  public:
    //!@{
    //! Type aliases
    using GeoTestMap = std::map<unsigned int, Volume>;
    //!@}

    // Construct empty
    GeometryStore();
    // Constructor from world physical volume
    GeometryStore(const G4VPhysicalVolume* world_physical_volume);
    // Default destructor
    ~GeometryStore() = default;

    // Populate GeoTestMap from physical volume
    void operator()(const G4VPhysicalVolume* world_physical_volume);

    // Getter for the constructed map
    const GeoTestMap& get_map() const;

    // Save a text output file with the data loaded in this->ids_volumes_
    void save(const std::string filename);

  private:
    // Recursive loop over logical volumes
    void loop_volumes(const G4LogicalVolume* logical_volume);

  private:
    // Map volume id with volume information
    GeoTestMap ids_volumes_;
};

//---------------------------------------------------------------------------//
// Free functions
//---------------------------------------------------------------------------//

// Define operator << to print a full table with GeoTestMap data.
std::ostream&
operator<<(std::ostream& os, const GeometryStore::GeoTestMap& map);
