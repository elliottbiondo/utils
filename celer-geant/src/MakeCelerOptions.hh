//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/MakeCelerOptions.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4Electron.hh>
#include <G4Gamma.hh>
#include <G4MuonMinus.hh>
#include <G4MuonPlus.hh>
#include <G4Neutron.hh>
#include <G4Positron.hh>
#include <accel/AlongStepFactory.hh>
#include <accel/SetupOptions.hh>
#include <accel/TrackingManagerConstructor.hh>
#include <celeritas/phys/PDGNumber.hh>
#include <corecel/Assert.hh>

#include "JsonReader.hh"

//---------------------------------------------------------------------------/
/*!
 * Load vector of \c G4ParticleDefinition from list of PDGs.
 */
celeritas::SetupOptions::VecG4PD from_pdgs(std::vector<int> input)
{
    using celeritas::PDGNumber;
    static std::unordered_map<PDGNumber, G4ParticleDefinition*> supported = {
        {celeritas::pdg::gamma(), G4Gamma::Definition()},
        {celeritas::pdg::electron(), G4Electron::Definition()},
        {celeritas::pdg::positron(), G4Positron::Definition()},
        {celeritas::pdg::mu_minus(), G4MuonMinus::Definition()},
        {celeritas::pdg::mu_plus(), G4MuonPlus::Definition()},
    };

    CELER_VALIDATE(!input.empty(),
                   << "Celeritas \"offload_particles\" option is present but "
                      "empty. Specify PDGs or remove it to use the Celeritas "
                      "default list.");
    celeritas::SetupOptions::VecG4PD result;
    for (auto pdg : input)
    {
        auto it = supported.find(PDGNumber{pdg});
        CELER_VALIDATE(it != supported.end(),
                       << "PDG '" << pdg << "' not available");
        result.push_back(it->second);
    }
    return result;
}

//---------------------------------------------------------------------------/
/*!
 * Celeritas runtime options.
 */
celeritas::SetupOptions MakeCelerOptions()
{
    using PDG = int;
    using VecPDG = std::vector<PDG>;

    JsonReader::Validate(JsonReader::Instance(), "celeritas");
    auto const& json = JsonReader::Instance().at("celeritas");

    celeritas::SetupOptions opts;
    JsonReader::Validate(json, "max_num_tracks");
    opts.max_num_tracks = json.at("max_num_tracks").get<size_t>();

    JsonReader::Validate(json, "initializer_capacity");
    opts.initializer_capacity = json.at("initializer_capacity").get<size_t>();

    if (json.contains("offload_particles"))
    {
        opts.offload_particles
            = from_pdgs(json.at("offload_particles").get<VecPDG>());
    }
    else
    {
        CELER_LOG(info)
            << "Celeritas' \"offload_particles\" option not present. "
               "Using default list.";
    }

    opts.sd.ignore_zero_deposition = false;

    // Set along-step factory with zero field
    opts.make_along_step = celeritas::UniformAlongStepFactory();

    return opts;
}
