//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file GeometryStore.hh
//---------------------------------------------------------------------------//
#include "GeometryStore.hh"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct.
 */
GeometryStore::GeometryStore()
    : phys_vol_store_(G4PhysicalVolumeStore::GetInstance())
{
    this->loop_volumes();
}

//---------------------------------------------------------------------------//
/*!
 * Get volumes vector.
 */
std::vector<Volume> GeometryStore::get_volumes() const
{
    return volumes_;
}

//---------------------------------------------------------------------------//
/*!
 * Save data stored in this->ids_volumes map as a text output file.
 */
void GeometryStore::save(const std::string filename)
{
    std::ofstream output;
    output.open(filename);
    output << volumes_ << std::endl;
    output.close();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Loop to store all physical volumes.
 */
void GeometryStore::loop_volumes()
{
    for (const auto& phys_vol : *phys_vol_store_)
    {
        const auto& logical_volume = phys_vol->GetLogicalVolume();

        auto volume                 = Volume();
        volume.physical_volume_name = phys_vol->GetName();
        volume.logical_volume_name  = logical_volume->GetName();
        volume.volume_id            = phys_vol->GetInstanceID();
        volume.material_id   = logical_volume->GetMaterial()->GetIndex();
        volume.material_name = logical_volume->GetMaterial()->GetName();
        volume.copy_num      = phys_vol->GetCopyNo();

        EAxis  dummy_a;
        double dummy_b, dummy_c;
        bool   dummy_d;
        phys_vol->GetReplicationData(
            dummy_a, volume.num_replicas, dummy_b, dummy_c, dummy_d);
        volumes_.push_back(volume);
    }
}

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Define operator << to print a full table with the GeoTestMap data.
 */
std::ostream& operator<<(std::ostream& os, std::vector<Volume> list)
{
    size_t width_ids      = 10;
    size_t width_pv       = 12;
    size_t width_lv       = 12;
    size_t width_material = 8;

    for (const auto& val : list)
    {
        width_ids = std::max(width_ids, std::to_string(list.size()).size());
        width_pv  = std::max(width_pv, val.physical_volume_name.size());
        width_lv  = std::max(width_lv, val.logical_volume_name.size());
        width_material = std::max(width_material, val.material_name.size());
    }

    // Title
    os << std::endl;
    os << "| " << std::left << std::setw(width_ids) << "Vol ID"
       << " | " << std::left << std::setw(width_ids) << "Copy Num"
       << " | " << std::left << std::setw(width_ids) << "Replica"
       << " | " << std::left << std::setw(width_ids) << "Mat ID"
       << " | " << std::setw(width_material) << "Material"
       << " | " << std::setw(width_pv) << "Phys. volume"
       << " | " << std::setw(width_lv) << "Log. volume"
       << " |" << std::endl;

    // Dashed line
    os << "| ";

    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_material; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_pv; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_lv; i++)
        os << "-";

    os << " | ";
    os << std::endl;

    // Table content
    for (const auto& val : list)
    {
        os << "| " << std::left << std::setw(width_ids) << val.volume_id
           << " | " << std::left << std::setw(width_ids) << val.copy_num
           << " | " << std::left << std::setw(width_ids) << val.num_replicas
           << " | " << std::left << std::setw(width_ids) << val.material_id
           << " | " << std::setw(width_material) << val.material_name << " | "
           << std::setw(width_pv) << val.physical_volume_name << " | "
           << std::setw(width_lv) << val.logical_volume_name << " |"
           << std::endl;
    }

    return os;
}
