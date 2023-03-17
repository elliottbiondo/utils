//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file vgapp.cc
//---------------------------------------------------------------------------//
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <VecGeom/gdml/Frontend.h>
#include <VecGeom/gdml/Middleware.h>
#include <VecGeom/management/GeoManager.h>

namespace vgapp
{
//---------------------------------------------------------------------------//
/*!
 * Store volume information.
 */
struct Volume
{
    std::string volume_name;
    std::string material_name;
};

// Map volume id and Volume
using VolumeMap = std::map<int, Volume>;

} // namespace vgapp

//---------------------------------------------------------------------------//
/*!
 * Define operator << to print a full table with the VolumeMap data.
 */
std::ostream& operator<<(std::ostream& os, const vgapp::VolumeMap& map)
{
    size_t width_ids      = 7;
    size_t width_volume   = 0;
    size_t width_material = 0;
    for (const auto& it : map)
    {
        width_volume = std::max(width_volume, it.second.volume_name.size());
        width_material
            = std::max(width_material, it.second.material_name.size());
    }

    // Titles line
    os << std::endl;
    os << "| " << std::left << std::setw(width_ids) << "Vol ID"
       << " | " << std::setw(width_material) << "Material"
       << " | " << std::setw(width_volume) << "Volume"
       << " |" << std::endl;

    // Dashed line
    os << "| ";
    for (int i = 0; i < width_ids; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_material; i++)
        os << "-";
    os << " | ";
    for (int i = 0; i < width_volume; i++)
        os << "-";
    os << " |";
    os << std::endl;

    // Table content
    for (const auto key : map)
    {
        os << "| " << std::left << std::setw(width_ids) << key.first << " | "
           << std::setw(width_material) << key.second.material_name << " | "
           << std::setw(width_volume) << key.second.volume_name << " |"
           << std::endl;
    }

    return os;
}

//---------------------------------------------------------------------------//
/*!
 * Minimal app that loads a gdml input file and generates a .md output file
 * with volume and material information.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        // Print help message
        std::cout << "Usage: " << argv[0] << " [input.gdml]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string gdml_input = argv[1];
    std::string md_filename
        = gdml_input.substr(0, gdml_input.find_last_of('.')) + ".md";
    std::cout << "Loading geometry and generating " << md_filename << "... ";
    std::cout.flush();

    // Load gdml
    vgdml::Parser parser;
    const auto    loaded = parser.Load(gdml_input, false);

    // Load material and volume information
    vgdml::Middleware::MaterialMap_t  material_map = loaded->GetMaterialMap();
    vgdml::Middleware::VolumeMatMap_t vol_mat_map  = loaded->GetVolumeMatMap();
    std::vector<vecgeom::LogicalVolume*> logical_volumes;
    vecgeom::GeoManager::Instance().GetAllLogicalVolumes(logical_volumes);

    vgapp::VolumeMap volume_map;
    for (const auto& vg_volume : logical_volumes)
    {
        vgapp::Volume volume;
        volume.volume_name   = vg_volume->GetLabel();
        volume.material_name = vol_mat_map.find(vg_volume->id())->second.name;
        volume_map.insert({vg_volume->id(), volume});
    }

    std::ofstream output;
    output.open(md_filename);
    output << volume_map << std::endl;
    output.close();
    std::cout << "Done" << std::endl;

    return EXIT_SUCCESS;
}
