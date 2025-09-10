//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/StackingAction.cc
//---------------------------------------------------------------------------//
#include "StackingAction.hh"

#include <algorithm>
#include <G4ClassificationOfNewTrack.hh>
#include <G4Track.hh>
#include <corecel/Assert.hh>

#include "JsonReader.hh"
//---------------------------------------------------------------------------//
/*!
 * Construct with list of valid PDGs from JSON.
 */
StackingAction::StackingAction() : G4UserStackingAction()
{
    JsonReader::Validate(JsonReader::Instance(), "celeritas");
    auto const& json = JsonReader::Instance().at("celeritas");
    if (json.contains("offload_particles"))
    {
        valid_pdgs_ = json.at("offload_particles").get<std::vector<PDG>>();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Assign \c fKill to all non-offloaded particles.
 */
G4ClassificationOfNewTrack
StackingAction::ClassifyNewTrack(G4Track* const track)
{
    auto is_valid = [this](PDG pdg) -> bool {
        return std::any_of(this->valid_pdgs_.begin(),
                           this->valid_pdgs_.end(),
                           [&pdg](PDG this_pdg) { return this_pdg == pdg; });
    };

    auto* pd = track->GetParticleDefinition();
    CELER_ASSERT(pd);

    return is_valid(pd->GetPDGEncoding()) ? fUrgent : fKill;
}
