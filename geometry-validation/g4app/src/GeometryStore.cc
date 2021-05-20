//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file GeometryStore.hh
//---------------------------------------------------------------------------//
#include "GeometryStore.hh"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <G4MaterialCutsCouple.hh>
#include <G4Material.hh>
#include <G4VSolid.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
GeometryStore::GeometryStore() = default;

//---------------------------------------------------------------------------//
/*!
 * Construct with physical volume.
 */
GeometryStore::GeometryStore(const G4VPhysicalVolume* world_physical_volume)
{
    this->loop_volumes(world_physical_volume->GetLogicalVolume());
}

//---------------------------------------------------------------------------//
/*!
 * Load volume map from a G4PhysicalVolume.
 */
void GeometryStore::operator()(const G4VPhysicalVolume* world_physical_volume)
{
    this->loop_volumes(world_physical_volume->GetLogicalVolume());
}
//---------------------------------------------------------------------------//
/*!
 * Get volume map.
 */
const GeometryStore::GeoTestMap& GeometryStore::get_map() const
{
    return ids_volumes_;
}

//---------------------------------------------------------------------------//
/*!
 * Save data stored in this->ids_volumes map as a text output file.
 */
void GeometryStore::save(const std::string filename)
{
    std::ofstream output;
    output.open(filename);
    output << ids_volumes_ << std::endl;
    output.close();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Recursive loop to store all logical volumes.
 */
void GeometryStore::loop_volumes(const G4LogicalVolume* logical_volume)
{
    // Add volume to the map
    Volume volume;
    volume.material_id = logical_volume->GetMaterialCutsCouple()->GetIndex();
    volume.material_name
        = logical_volume->GetMaterialCutsCouple()->GetMaterial()->GetName();
    volume.name       = logical_volume->GetName();
    volume.solid_name = logical_volume->GetSolid()->GetName();

    ids_volumes_.insert({logical_volume->GetInstanceID(), volume});

    // Recursive: repeat for every daughter volume, if any
    for (int i = 0; i < logical_volume->GetNoDaughters(); ++i)
    {
        loop_volumes(logical_volume->GetDaughter(i)->GetLogicalVolume());
    }
}

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Define operator << to print a full table with the GeoTestMap data.
 */
std::ostream& operator<<(std::ostream& os, const GeometryStore::GeoTestMap& map)
{
    size_t width_ids      = 6 + map.size() / 10;
    size_t width_volume    = 0;
    size_t width_material = 0;
    for (const auto& it : map)
    {
        width_volume = std::max(width_volume, it.second.name.size());
        width_material
            = std::max(width_material, it.second.material_name.size());
    }

    // Titles line
    os << std::endl;
    os << "| " << std::left << std::setw(width_ids) << "Vol ID"
       << " | " << std::left << std::setw(width_ids) << "Mat ID"
       << " | " << std::setw(width_material) << "Material"
       << " | " << std::setw(width_volume) << "Volume"
       << " |" << std::endl;

    // Dashed line
    os << "| ";
    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_material; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_volume; i++)
        os << "-";
    os << " | ";
    os << std::endl;

    // Table content
    for (const auto key : map)
    {
        os << "| " << std::left << std::setw(width_ids) << key.first << " | "
           << std::left << std::setw(width_ids) << key.second.material_id
           << " | " << std::setw(width_material) << key.second.material_name
           << " | " << std::setw(width_volume) << key.second.name << " |"
           << std::endl;
    }

    return os;
}
