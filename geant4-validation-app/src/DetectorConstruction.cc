//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.cc
//---------------------------------------------------------------------------//
#include "DetectorConstruction.hh"

#include <algorithm>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>

#include "JsonReader.hh"
#include "SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct by parsing the gdml input file.
 */
DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction()
{
    // Fetch GDML name and load physical world volume
    auto const& json = JsonReader::instance()->json();
    std::string gdml_input_file = json.at("geometry").get<std::string>();

    gdml_parser_.SetStripFlag(false);
    gdml_parser_.Read(gdml_input_file, false);
    phys_vol_world_.reset(gdml_parser_.GetWorldVolume());
}

//---------------------------------------------------------------------------//
/*!
 * Mandatory Construct function.
 */
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    return phys_vol_world_.release();
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive detectors and (TODO) magnetic field.
 */
void DetectorConstruction::ConstructSDandField()
{
    auto const& json_sim = JsonReader::instance()->json().at("simulation");
    if (json_sim.at("sensdet_info").get<bool>())
    {
        this->set_sd();
    }

    // TODO: Add magnetic field
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Set up sensitive detectors.
 */
void DetectorConstruction::set_sd()
{
    auto sd_manager = G4SDManager::GetSDMpointer();
    auto const aux_map = gdml_parser_.GetAuxMap();

    for (auto iter = aux_map->begin(); iter != aux_map->end(); iter++)
    {
        auto const& log_vol = iter->first;
        auto const& aux_list_type = iter->second;

        for (auto const& element : aux_list_type)
        {
            if (element.type != "SensDet")
            {
                // Not a sensitive detector auxiliary type; Skip
                continue;
            }

            // Add sensitive detector
            std::string sd_name = element.value;
            auto this_sd = new SensitiveDetector(sd_name, log_vol);
            sd_manager->AddNewDetector(this_sd);
            G4VUserDetectorConstruction::SetSensitiveDetector(
                log_vol->GetName(), this_sd);
        }
    }
}
