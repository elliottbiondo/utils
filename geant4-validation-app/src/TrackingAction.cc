//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file TrackingAction.cc
//---------------------------------------------------------------------------//
#include "TrackingAction.hh"

#include <algorithm>
#include <G4Track.hh>
#include <G4SystemOfUnits.hh>
#include <G4Electron.hh>
#include <G4Gamma.hh>
#include <G4Positron.hh>
#include <G4Track.hh>

#include "JsonReader.hh"
#include "Celeritas.hh"
#include <accel/ExceptionConverter.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct and set up ROOT I/O options.
 */
TrackingAction::TrackingAction()
    : G4UserTrackingAction(), root_io_(RootIO::instance())
{
    const auto& json_sim = JsonReader::instance()->json().at("simulation");
    offload_             = json_sim.at("offload").get<bool>();
    store_primaries_     = json_sim.at("primary_info").get<bool>();
    store_secondaries_   = json_sim.at("secondary_info").get<bool>();
}

//---------------------------------------------------------------------------//
/*!
 *  Pre-track simulation actions.
 */
void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    if (offload_)
    {
        static G4ParticleDefinition const* const allowed_particles[] = {
            G4Gamma::Gamma(),
            G4Electron::Electron(),
            G4Positron::Positron(),
        };

        if (std::find(std::begin(allowed_particles),
                      std::end(allowed_particles),
                      track->GetDefinition())
            != std::end(allowed_particles))
        {
            // Celeritas is transporting this track
            celeritas::ExceptionConverter call_g4exception{"celer0003"};
            CELER_TRY_HANDLE(CelerLocalTransporter().Push(*track),
                             call_g4exception);
            const_cast<G4Track*>(track)->SetTrackStatus(fStopAndKill);
        }
    }
    if (!root_io_)
    {
        return;
    }

    if (store_primaries_ || store_secondaries_)
    {
        root_io_->clear_track();
        root_io_->track_.vertex_global_time = track->GetGlobalTime() / s;
    }
}

//---------------------------------------------------------------------------//
/*!
 *  Post-track simulation actions.
 */
void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
    if (!root_io_)
    {
        return;
    }

    if (!store_primaries_ && !store_secondaries_)
    {
        // Tracks should not be stored
        return;
    }

    // Store total steps
    root_io_->steps_per_event_ += root_io_->track_.number_of_steps;

    // Store track information
    root_io_->track_.pdg = track->GetParticleDefinition()->GetPDGEncoding();
    root_io_->track_.id  = track->GetTrackID();
    root_io_->track_.parent_id        = track->GetParentID();
    root_io_->track_.length           = track->GetTrackLength() / cm;
    root_io_->track_.vertex_energy    = track->GetVertexKineticEnergy() / MeV;
    G4ThreeVector pos                 = track->GetVertexPosition() / cm;
    G4ThreeVector dir                 = track->GetVertexMomentumDirection();
    root_io_->track_.vertex_position  = {pos.x(), pos.y(), pos.z()};
    root_io_->track_.vertex_direction = {dir.x(), dir.y(), dir.z()};

    if (store_primaries_ && track->GetParentID() == 0)
    {
        // Fill primary information
        root_io_->event_.primaries.push_back(root_io_->track_);
    }

    else if (store_secondaries_ && track->GetParentID() != 0)
    {
        // Fill secondary information
        root_io_->event_.secondaries.push_back(root_io_->track_);
    }

    // Store data limits information
    root_io_->data_limits_.max_vertex
        = {std::max(pos.x(), root_io_->data_limits_.max_vertex.x),
           std::max(pos.y(), root_io_->data_limits_.max_vertex.y),
           std::max(pos.z(), root_io_->data_limits_.max_vertex.z)};

    root_io_->data_limits_.min_vertex
        = {std::min(pos.x(), root_io_->data_limits_.min_vertex.x),
           std::min(pos.y(), root_io_->data_limits_.min_vertex.y),
           std::min(pos.z(), root_io_->data_limits_.min_vertex.z)};

    root_io_->data_limits_.max_trk_length = std::max(
        root_io_->data_limits_.max_trk_length, root_io_->track_.length);

    if (store_primaries_ && track->GetParentID() == 0)
    {
        // Primary info
        root_io_->data_limits_.max_primary_energy
            = std::max(root_io_->track_.vertex_energy,
                       root_io_->data_limits_.max_primary_energy);

        root_io_->data_limits_.max_primary_num_steps
            = std::max(root_io_->event_.primaries.back().number_of_steps,
                       root_io_->data_limits_.max_primary_num_steps);
    }

    else if (store_secondaries_ && track->GetParentID() != 0)
    {
        // Secondary info
        root_io_->data_limits_.max_secondary_energy
            = std::max(root_io_->track_.vertex_energy,
                       root_io_->data_limits_.max_secondary_energy);

        root_io_->data_limits_.max_secondary_num_steps
            = std::max(root_io_->event_.secondaries.back().number_of_steps,
                       root_io_->data_limits_.max_secondary_num_steps);
    }
}
