//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/DetectorConstruction.cc
//---------------------------------------------------------------------------//
#include "DetectorConstruction.hh"

#include <G4LogicalVolume.hh>
#include <G4SDManager.hh>
#include <corecel/Assert.hh>
#include <corecel/io/Logger.hh>

#include "SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with filename.
 */
DetectorConstruction::DetectorConstruction(std::string gdml_filename)
    : G4VUserDetectorConstruction()
{
    CELER_VALIDATE(!gdml_filename.empty(), << "GDML filename is empty");
    parser_.SetStripFlag(false);
    parser_.SetOverlapCheck(false);
    parser_.Read(gdml_filename, false);
}

//---------------------------------------------------------------------------//
/*!
 * Construct world geometry.
 */
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    return parser_.GetWorldVolume();
}

//---------------------------------------------------------------------------//
/*!
 * Initialize sensitive detectors on all threads.
 */
void DetectorConstruction::ConstructSDandField()
{
    if (false /* field */)
    {
        //! \todo Initialize field
        CELER_LOG(status) << "Initializing magnetic field";
    }

    this->InitializeSensitiveDetectors();
}

//---------------------------------------------------------------------------//
/*!
 * Find and set sensitive detectors.
 *
 * \todo Use a physical volume store to force all volumes to be scored.
 */
void DetectorConstruction::InitializeSensitiveDetectors()
{
    CELER_LOG(status) << "Initializing sensitive detectors";
    auto sd_manager = G4SDManager::GetSDMpointer();
    CELER_ASSERT(sd_manager);
    auto const aux_map = parser_.GetAuxMap();
    CELER_ASSERT(aux_map);

    for (auto iter = aux_map->begin(); iter != aux_map->end(); iter++)
    {
        auto const& log_vol = iter->first;
        auto const& aux_list_type = iter->second;

        for (auto const& element : aux_list_type)
        {
            if (element.type != "SensDet")
            {
                // Skip non-sensitive detector auxiliary types
                continue;
            }

            // Add sensitive detector
            std::string sd_name = element.value;
            auto this_sd = std::make_unique<SensitiveDetector>(sd_name);
            G4VUserDetectorConstruction::SetSensitiveDetector(
                log_vol->GetName(), this_sd.get());
            sd_manager->AddNewDetector(this_sd.release());
            CELER_LOG(debug)
                << "Inserted " << sd_name << " as sensitive detector";
        }
    }
}
