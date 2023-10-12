//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SensitiveDetector.cc
//---------------------------------------------------------------------------//
#include "SensitiveDetector.hh"

#include <algorithm>
#include <G4SystemOfUnits.hh>
#include <G4VProcess.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4ThreeVector.hh>
#include <G4Step.hh>

#include "RootIO.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with sensitive detector name.
 */
SensitiveDetector::SensitiveDetector(G4String         sd_name,
                                     G4LogicalVolume* logical_volume)
    : G4VSensitiveDetector(sd_name)
    , sd_name_(sd_name)
    , root_io_(RootIO::instance())
{
    if (!root_io_)
    {
        return;
    }

    const auto* phys_vol_store = G4PhysicalVolumeStore::GetInstance();
    const auto& log_vol_name   = logical_volume->GetName();

    for (const auto& phys_vol : *phys_vol_store)
    {
        if (phys_vol->GetLogicalVolume()->GetName() == log_vol_name)
        {
            rootdata::SensDetGdml sd_gdml;
            sd_gdml.name        = sd_name;
            sd_gdml.copy_number = phys_vol->GetCopyNo();

            root_io_->add_sd(sd_gdml);
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Called at the beginning of each event.
 */
void SensitiveDetector::Initialize(G4HCofThisEvent*)
{
    if (!root_io_)
    {
        return;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Mandatory function called at each step.
 */
G4bool SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    if (!root_io_)
    {
        return false;
    }

    const double energy_dep = step->GetTotalEnergyDeposit() / MeV;

    if (!energy_dep)
    {
        // No energy deposition; nothing to do
        return false;
    }

    auto process_id = rootdata::ProcessId::not_mapped;
    if (G4StepPoint* post_step = step->GetPostStepPoint())
    {
        if (auto* process = post_step->GetProcessDefinedStep())
        {
            rootdata::to_process_name_id(process->GetProcessName());
            if (process_id == rootdata::ProcessId::transportation)
            {
                return true;
            }
        }
    }

    rootdata::SensDetGdml sd_gdml;
    sd_gdml.name        = sd_name_;
    sd_gdml.copy_number = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetCopyNo();

    // Find correct index in root_io->event_.sensitive_detectors
    const auto& iter = root_io_->sdgdml_sensdetidx_.find(sd_gdml);
    assert(iter != root_io_->sdgdml_sensdetidx_.end());
    auto idx = iter->second;

    // Add scoring to sensitive detector data
    auto& sensdet_vec = root_io_->event_.sensitive_detectors;
    sensdet_vec[idx].energy_deposition += energy_dep;
    sensdet_vec[idx].number_of_steps++;

    rootdata::SensDetScoreData::map_adder(
        sensdet_vec[idx].process_counter, process_id, 1);
    rootdata::SensDetScoreData::map_adder(
        sensdet_vec[idx].process_edep, process_id, energy_dep);

    return true;
}

//---------------------------------------------------------------------------//
/*!
 * Optional function called at the end of event.
 */
void SensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
    if (!root_io_)
    {
        return;
    }

    // Store data limit information
    const auto& sd_vector = root_io_->event_.sensitive_detectors;
    auto&       lim       = root_io_->data_limits_;
    for (const auto& sd : sd_vector)
    {
        lim.max_sd_energy = std::max(sd.energy_deposition, lim.max_sd_energy);
        lim.max_sd_num_steps
            = std::max(sd.number_of_steps, lim.max_sd_num_steps);
    }
}
