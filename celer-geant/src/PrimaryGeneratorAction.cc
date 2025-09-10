//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/PrimaryGeneratorAction.cc
//---------------------------------------------------------------------------//
#include "PrimaryGeneratorAction.hh"

#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4SystemOfUnits.hh>
#include <corecel/Assert.hh>

#include "DetectorConstruction.hh"
#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Generate primaries.
 */
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    CELER_EXPECT(event);
    auto const& json = JsonReader::Instance();
    JsonReader::Validate(json, "particle_gun");

    auto const& pg = json.at("particle_gun");
    JsonReader::Validate(pg, "pdg");
    JsonReader::Validate(pg, "energy");
    JsonReader::Validate(pg, "vertex");
    JsonReader::Validate(pg, "direction");

    auto const pdg = pg.at("pdg").get<int>();
    auto const energy = pg.at("energy").get<double>();
    G4ThreeVector vertex(pg.at("vertex")[0].get<double>(),
                         pg.at("vertex")[1].get<double>(),
                         pg.at("vertex")[2].get<double>());
    vertex *= cm;  // Convert to cm
    G4ThreeVector direction(pg.at("direction")[0].get<double>(),
                            pg.at("direction")[1].get<double>(),
                            pg.at("direction")[2].get<double>());

    G4ParticleGun particle_gun;
    particle_gun.SetParticleDefinition(
        G4ParticleTable::GetParticleTable()->FindParticle(pdg));
    particle_gun.SetParticleEnergy(energy);
    particle_gun.SetParticlePosition(vertex);
    particle_gun.SetParticleMomentumDirection(direction);
    particle_gun.GeneratePrimaryVertex(event);
}
