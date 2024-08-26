//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.hh
//! \brief Define the particle gun.
//---------------------------------------------------------------------------//
#pragma once

#include <memory>
#include <G4Event.hh>
#include <G4ParticleGun.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

//---------------------------------------------------------------------------//
/*!
 * Set and run the particle gun.
 */
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    // Construct by setting up the particle gun or loading the hepmc3 file
    PrimaryGeneratorAction();

    // Generate events
    void GeneratePrimaries(G4Event* event) override;

  private:
    // Set up particle gun if json's hepmc3 field is empty
    void set_particle_gun();

    // Set up hepmc3 input if provided
    void set_hepmc3();

  private:
    std::shared_ptr<G4ParticleGun> particle_gun_;
    bool is_hepmc3_;
};
