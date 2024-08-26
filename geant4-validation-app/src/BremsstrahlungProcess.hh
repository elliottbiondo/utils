//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file BremsstrahlungProcess.hh
//! \brief Bremsstrahlung process class.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4VEnergyLossProcess.hh>

//---------------------------------------------------------------------------//
/*!
 * Electron/positron Bremsstrahlung process class derived from
 * \c G4eBremsstrahlung . The need for a new process class is to add the option
 * to manually select individual models.
 */
class BremsstrahlungProcess : public G4VEnergyLossProcess
{
  public:
    enum class ModelSelection
    {
        seltzer_berger,
        relativistic,
        all
    };

    // Construct with model selection
    BremsstrahlungProcess(ModelSelection selection);

    // Empty destructor
    ~BremsstrahlungProcess();

    // True for electrons and positrons
    bool IsApplicable(G4ParticleDefinition const& particle) final;

    // Print documentation
    void ProcessDescription(std::ostream&) const override;

  protected:
    // Initialise process by constructing selected models
    void InitialiseEnergyLossProcess(G4ParticleDefinition const*,
                                     G4ParticleDefinition const*) override;
    // Print class parameters
    void StreamProcessInfo(std::ostream& output) const override;

  protected:
    bool is_initialized_;

  private:
    // Hide assignment operator
    BremsstrahlungProcess& operator=(BremsstrahlungProcess const& right)
        = delete;
    BremsstrahlungProcess(BremsstrahlungProcess const&) = delete;

  private:
    ModelSelection model_selection_;
};
