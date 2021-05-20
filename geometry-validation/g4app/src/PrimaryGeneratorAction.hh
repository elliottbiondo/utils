//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PrimaryGeneratorAction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <memory>

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4Event.hh>
#include <G4ParticleGun.hh>

//---------------------------------------------------------------------------//
/*!
 * Mandatory Geant4 class implementation.
 */
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    // Construct by setting up a minimal particle gun 
    PrimaryGeneratorAction();
    // Default destructor
    ~PrimaryGeneratorAction();

    // Implement mandatory virtual function
    void GeneratePrimaries(G4Event* event) override;

  private:
    std::unique_ptr<G4ParticleGun> particle_gun_;
};
