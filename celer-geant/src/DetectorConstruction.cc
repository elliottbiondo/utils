//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/DetectorConstruction.cc
//---------------------------------------------------------------------------//
#include "DetectorConstruction.hh"

#include <G4LogicalVolume.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4SDManager.hh>
#include <corecel/Assert.hh>
#include <corecel/io/Logger.hh>

#include "JsonReader.hh"
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

    auto const& json = JsonReader::Instance();
    JsonReader::Validate(json, "all_volumes_sensitive");
    if (json.at("all_volumes_sensitive").get<bool>())
    {
        this->MakeAllVolumesSensitive();
    }
    else
    {
        this->InitializeSensitiveDetectors();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Find and set sensitive detectors.
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

//---------------------------------------------------------------------------//
/*!
 * Set all physical volumes as sensitive detectors.
 */
void DetectorConstruction::MakeAllVolumesSensitive()
{
    CELER_LOG_LOCAL(status)
        << "Initializing all physical volumes as sensitive "
           "detectors";

    auto sd_manager = G4SDManager::GetSDMpointer();
    CELER_ASSERT(sd_manager);
    auto const& physvol_store = *G4PhysicalVolumeStore::GetInstance();
    CELER_ASSERT(!physvol_store.empty());

    for (auto const& physvol : physvol_store)
    {
        CELER_ASSERT(physvol);
        auto const* logvol = physvol->GetLogicalVolume();
        CELER_ASSERT(logvol);

        std::string sd_name = logvol->GetName() + "_sd";
        auto this_sd = std::make_unique<SensitiveDetector>(sd_name);
        G4VUserDetectorConstruction::SetSensitiveDetector(logvol->GetName(),
                                                          this_sd.get());
        sd_manager->AddNewDetector(this_sd.release());
        CELER_LOG(debug) << "Initialized " << logvol->GetName()
                         << " as sensitive detector with name '" << sd_name
                         << "'";
    }
}
