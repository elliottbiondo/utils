//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/PrimaryGeneratorAction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4Event.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

//---------------------------------------------------------------------------//
/*!
 * Generate primaries.
 */
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
  public:
    //! Construct empty
    PrimaryGeneratorAction() = default;

    //! Place primaries in the event simulation
    void GeneratePrimaries(G4Event* event) final;
};
