//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.hh
//! \brief Build the physics list.
//---------------------------------------------------------------------------//
#pragma once

#include <G4VUserPhysicsList.hh>

//---------------------------------------------------------------------------//
/*!
 * Constructs empty physics list for a minimal Geant4 initialization.
 */
class PhysicsList : public G4VUserPhysicsList
{
  public:
    // Construct physics list with given range cuts in mm
    PhysicsList(double range_cuts);

    // Set up minimal E.M. particle list
    void ConstructParticle() final;
    // Set up process list
    void ConstructProcess() final;
    // Set secondary production cuts
    void SetCuts() final;

    // Return Geant4's default world region name
    static char const* default_region_name()
    {
        return "DefaultRegionForTheWorld";
    }

  private:
    double range_cuts_;  // [mm]
};
