//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.cc
//---------------------------------------------------------------------------//
#include "PrimaryGeneratorAction.hh"

#include <G4ParticleTable.hh>
#include <G4SystemOfUnits.hh>

#include "HepMC3Reader.hh"
#include "JsonReader.hh"
#include "RootData.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct with json input information.
 */
PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction()
{
    auto const json_input = JsonReader::instance()->json();
    std::string hepmc3_input
        = json_input.at("simulation").at("hepmc3").get<std::string>();

    is_hepmc3_ = hepmc3_input.empty() ? false : true;

    if (!is_hepmc3_)
    {
        // No HepMC3 input file provided; use particle gun
        this->set_particle_gun();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Generate primary at each new event.
 */
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    if (is_hepmc3_)
    {
        auto const hepmc3 = HepMC3Reader::instance();

        // Read event and set appropriate conditions
        hepmc3->read_event();
        auto const primaries = hepmc3->event_primaries();
        for (auto const& primary : primaries)
        {
            auto* g4_particle_def
                = G4ParticleTable::GetParticleTable()->FindParticle(
                    primary.pdg);

            if (!g4_particle_def)
            {
                // Particle definition not available
                std::cerr << "Warning: In event " << hepmc3->event_number()
                          << ", primary PGD " << primary.pdg
                          << " not found in G4ParticleTable. Skipping..."
                          << std::endl;
                continue;
            }

            G4ParticleGun particle_gun;
            particle_gun.SetParticleDefinition(g4_particle_def);
            particle_gun.SetParticleEnergy(primary.energy);
            particle_gun.SetParticlePosition(G4ThreeVector(
                primary.vertex[0], primary.vertex[1], primary.vertex[2]));

            G4ThreeVector direction(
                primary.momentum[0], primary.momentum[1], primary.momentum[2]);
            direction = direction.unit();
            particle_gun.SetParticleMomentumDirection(direction);

            particle_gun.GeneratePrimaryVertex(event);
        }
    }
    else
    {
        particle_gun_->GeneratePrimaryVertex(event);
    }
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Set up particle gun.
 */
void PrimaryGeneratorAction::set_particle_gun()
{
    auto const json_input = JsonReader::instance()->json();
    auto const part_gun = json_input.at("simulation").at("particle_gun");

    HepMC3Reader::Primary primary;
    primary.pdg = part_gun.at("pdg").get<int>();
    primary.energy = part_gun.at("energy").get<double>() * MeV;
    G4ThreeVector vertex(part_gun.at("vertex")[0].get<double>() * cm,
                         part_gun.at("vertex")[1].get<double>() * cm,
                         part_gun.at("vertex")[2].get<double>() * cm);
    G4ThreeVector direction(part_gun.at("direction")[0].get<double>(),
                            part_gun.at("direction")[1].get<double>(),
                            part_gun.at("direction")[2].get<double>());
    direction = direction.unit();

    // Create the particle gun
    int const number_of_particles = 1;
    particle_gun_ = std::make_shared<G4ParticleGun>(number_of_particles);

    // Particle gun setup
    particle_gun_->SetParticleDefinition(
        G4ParticleTable::GetParticleTable()->FindParticle(primary.pdg));
    particle_gun_->SetParticleMomentumDirection(direction);
    particle_gun_->SetParticleEnergy(primary.energy);
    particle_gun_->SetParticlePosition(vertex);
}
