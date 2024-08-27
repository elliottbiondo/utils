//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.hh
//! \brief Build the physics list.
//---------------------------------------------------------------------------//
#pragma once

#include <map>
#include <G4VUserPhysicsList.hh>

//---------------------------------------------------------------------------//
/*!
 * Constructs particles and processes to be used in the simulation run.
 */
class PhysicsList : public G4VUserPhysicsList
{
  public:
    // Load physics list from Json input file
    PhysicsList();

    // Set up minimal E.M. particle list
    void ConstructParticle() override;
    // Set up process list defined in the json input file
    void ConstructProcess() override;

  private:
    // Add EM processes for photons
    void add_gamma_processes();
    // Add EM processes for electrons and positrons
    void add_e_processes(G4ParticleDefinition* particle);
    // Add optical physics processes to all applicable particles
    void add_optical_processes();

  private:
    // Map of selected processes; booleans are updated at construction time
    std::map<std::string, bool> selected_processes_
        = {{"compton_scattering", false},
           {"photoelectric", false},
           {"rayleigh_scattering", false},
           {"gamma_conversion", false},
           {"positron_annihilation", false},
           {"bremsstrahlung", false},
           {"e_ionization", false},
           {"coulomb_scattering", false},
           {"multiple_scattering_low", false},
           {"multiple_scattering_high", false},
           {"scintillation", false},
           {"cerenkov", false},
           {"optical_rayleigh", false}};
};
