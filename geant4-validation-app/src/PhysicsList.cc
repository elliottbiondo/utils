//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.cc
//---------------------------------------------------------------------------//
#include "PhysicsList.hh"

#include "JsonReader.hh"
#include "G4appMacros.hh"

#include <G4ProcessManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4PhysicsListHelper.hh>
#include <G4TransportationManager.hh>

#include <G4Gamma.hh>
#include <G4Electron.hh>
#include <G4Positron.hh>
#include <G4Proton.hh>
#include <G4GenericIon.hh>

#include "BremsstrahlungProcess.hh"

#include <G4PhotoElectricEffect.hh>
#include <G4LivermorePhotoElectricModel.hh>

#include <G4ComptonScattering.hh>

#include <G4GammaConversion.hh>
#include <G4PairProductionRelModel.hh>

#include <G4CoulombScattering.hh>
#include <G4eCoulombScatteringModel.hh>

#include <G4eIonisation.hh>
#include <G4MollerBhabhaModel.hh>

#include <G4RayleighScattering.hh>
#include <G4LivermoreRayleighModel.hh>

#include <G4eplusAnnihilation.hh>
#include <G4eeToTwoGammaModel.hh>

#include <G4eMultipleScattering.hh>
#include <G4UrbanMscModel.hh>
#include <G4WentzelVIModel.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct, load list of physics processes, and set verbosity.
 */
PhysicsList::PhysicsList() : G4VUserPhysicsList()
{
    const auto  json     = JsonReader::instance()->json();
    const auto& json_sim = json.at("simulation");

    // Update selected processes according to the json
    for (auto pair : selected_processes_)
    {
        selected_processes_.at(pair.first)
            = json.at("physics").at(pair.first).get<bool>();
    }

    auto em_parameters = G4EmParameters::Instance();

    bool eloss_fluct = json_sim.at("eloss_fluctuation").get<bool>();
    em_parameters->SetLossFluctuations(eloss_fluct);

    // TODO implement binning
    // em_parameters->SetNumberOfBinsPerDecade(nbins);

    // Set verbosity level
    int level = json.at("verbosity").at("PhysicsList").get<int>();
    em_parameters->SetVerbose(level);

    // Set spline; TODO use G4VEmProcess::SetSplineFlag for v11
#if G4_V10
    bool spline = json_sim.at("spline").get<bool>();
    em_parameters->SetSpline(spline);
#endif
}

//---------------------------------------------------------------------------//
/*!
 * Build list of available particles.
 *
 * The minimal EM set can be built by using
 * \c G4EmBuilder::ConstructMinimalEmSet();
 * and includes gamma, e+, e-, mu+, mu-, pi+, pi-, K+, K-, p, pbar, deuteron,
 * triton, He3, alpha, and generic ion, along with Geant4's pseudo-particles
 * geantino and charged geantino.
 *
 * Currently only instantiating e+, e-, gamma, and proton. The latter is needed
 * for msc and production cut tables.
 */
void PhysicsList::ConstructParticle()
{
    G4Gamma::GammaDefinition();
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
    G4Proton::ProtonDefinition();

    const bool msc_on
        = (selected_processes_.find("multiple_scattering_low")->second
           || selected_processes_.find("multiple_scattering_high")->second);

    const bool coulomb_on
        = selected_processes_.find("coulomb_scattering")->second;

    if (msc_on || coulomb_on)
    {
        G4GenericIon::GenericIonDefinition();
    }
}

//---------------------------------------------------------------------------//
/*!
 * Add transportation and selected processes.
 */
void PhysicsList::ConstructProcess()
{
    // Add transportation
    G4VUserPhysicsList::AddTransportation();

    // Add EM processes for photons, electrons, and positrons
    this->add_gamma_processes();
    this->add_e_processes(G4Electron::Electron());
    this->add_e_processes(G4Positron::Positron());
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Add EM processes for photons.
 *
 * | Processes            | Model class                   |
 * | -------------------- | ----------------------------- |
 * | Compton scattering   | G4KleinNishinaCompton         |
 * | Photoelectric effect | G4LivermorePhotoElectricModel |
 * | Rayleigh scattering  | G4LivermoreRayleighModel      |
 * | Gamma conversion     | G4PairProductionRelModel      |
 */
void PhysicsList::add_gamma_processes()
{
    auto       physics_list = G4PhysicsListHelper::GetPhysicsListHelper();
    const auto gamma        = G4Gamma::Gamma();

    if (selected_processes_.find("compton_scattering")->second)
    {
        // Compton Scattering: G4KleinNishinaCompton
        auto compton_scattering = std::make_unique<G4ComptonScattering>();
        physics_list->RegisterProcess(compton_scattering.release(), gamma);
    }

    if (selected_processes_.find("photoelectric")->second)
    {
        // Photoelectric effect: G4LivermorePhotoElectricModel
        auto photoelectrict_effect = std::make_unique<G4PhotoElectricEffect>();
        photoelectrict_effect->SetEmModel(new G4LivermorePhotoElectricModel());
        physics_list->RegisterProcess(photoelectrict_effect.release(), gamma);
    }

    if (selected_processes_.find("rayleigh_scattering")->second)
    {
        // Rayleigh: G4LivermoreRayleighModel
        physics_list->RegisterProcess(new G4RayleighScattering(), gamma);
    }

    if (selected_processes_.find("gamma_conversion")->second)
    {
        // Gamma conversion: G4PairProductionRelModel
        auto gamma_conversion = std::make_unique<G4GammaConversion>();
        gamma_conversion->SetEmModel(new G4PairProductionRelModel());
        physics_list->RegisterProcess(gamma_conversion.release(), gamma);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Add EM processes for electrons and positrons.
 *
 * | Processes                    | Model class               |
 * | ---------------------------- | --------------------------|
 * | Pair annihilation            | G4eeToTwoGammaModel       |
 * | Ionization                   | G4MollerBhabhaModel       |
 * | Bremsstrahlung (low E)       | G4SeltzerBergerModel      |
 * | Bremsstrahlung (high E)      | G4eBremsstrahlungRelModel |
 * | Coulomb scattering           | G4eCoulombScatteringModel |
 * | Multiple scattering (low E)  | G4UrbanMscModel           |
 * | Multiple scattering (high E) | G4WentzelVIModel          |
 *
 * \note Bremsstrahlung models are selected manually at compile time using
 * \c BremsModelSelection and need to be updated accordingly.
 */
void PhysicsList::add_e_processes(G4ParticleDefinition* particle)
{
    auto physics_list = G4PhysicsListHelper::GetPhysicsListHelper();

    if (selected_processes_.find("positron_annihilation")->second
        && particle == G4Positron::Positron())
    {
        // e+e- annihilation: G4eeToTwoGammaModel
        physics_list->RegisterProcess(new G4eplusAnnihilation(), particle);
    }

    if (selected_processes_.find("e_ionization")->second)
    {
        // e-e+ ionization: G4MollerBhabhaModel
        auto ionization_process = std::make_unique<G4eIonisation>();
        ionization_process->SetEmModel(new G4MollerBhabhaModel());
        physics_list->RegisterProcess(ionization_process.release(), particle);
    }

    if (selected_processes_.find("bremsstrahlung")->second)
    {
        // Bremmstrahlung: G4SeltzerBergerModel + G4eBremsstrahlungRelModel
        auto models        = BremsstrahlungProcess::ModelSelection::all;
        auto brems_process = std::make_unique<BremsstrahlungProcess>(models);
        physics_list->RegisterProcess(brems_process.release(), particle);

        if (!selected_processes_.find("e_ionization")->second)
        {
            // If ionization is turned off, activate the along-step "do it" for
            // bremsstrahlung *after* the process has been registered and set
            // the order to be the same as the default post-step order. See \c
            // G4PhysicsListHelper and the ordering parameter table for more
            // information on which "do its" are activated for each process and
            // the default process ordering.
            auto* process_manager = particle->GetProcessManager();
            auto* bremsstrahlung  = dynamic_cast<BremsstrahlungProcess*>(
                process_manager->GetProcess("eBrem"));
            auto order = process_manager->GetProcessOrdering(
                bremsstrahlung, G4ProcessVectorDoItIndex::idxPostStep);
            process_manager->SetProcessOrdering(
                bremsstrahlung, G4ProcessVectorDoItIndex::idxAlongStep, order);

            // Let this process be a candidate for range limiting the step
            bremsstrahlung->SetIonisation(true);
        }
    }

    if (selected_processes_.find("coulomb_scattering")->second)
    {
        // Coulomb scattering: G4eCoulombScatteringModel
        const double msc_threshold_energy
            = G4EmParameters::Instance()->MscEnergyLimit();

        auto coulomb_process = std::make_unique<G4CoulombScattering>();
        auto coulomb_model   = std::make_unique<G4eCoulombScatteringModel>();
        coulomb_process->SetMinKinEnergy(msc_threshold_energy);
        coulomb_model->SetLowEnergyLimit(msc_threshold_energy);
        coulomb_model->SetActivationLowEnergyLimit(msc_threshold_energy);
        coulomb_process->SetEmModel(coulomb_model.release());
        physics_list->RegisterProcess(coulomb_process.release(), particle);
    }

    const bool msc_low
        = selected_processes_.find("multiple_scattering_low")->second;
    const bool msc_high
        = selected_processes_.find("multiple_scattering_high")->second;

    if (msc_low || msc_high)
    {
        // Multiple scattering
        const double msc_threshold_energy
            = G4EmParameters::Instance()->MscEnergyLimit();
        auto msc_process = std::make_unique<G4eMultipleScattering>();

        if (msc_low)
        {
            // Urban model
            auto urban_model = std::make_unique<G4UrbanMscModel>();
            urban_model->SetHighEnergyLimit(msc_threshold_energy);
            urban_model->SetActivationHighEnergyLimit(msc_threshold_energy);
            msc_process->SetEmModel(urban_model.release());
        }

        if (msc_high)
        {
            // WentzelVI model
            auto wentzelvi_model = std::make_unique<G4WentzelVIModel>();
            wentzelvi_model->SetLowEnergyLimit(msc_threshold_energy);
            wentzelvi_model->SetActivationLowEnergyLimit(msc_threshold_energy);
            msc_process->SetEmModel(wentzelvi_model.release());
        }

        physics_list->RegisterProcess(msc_process.release(), particle);
    }
}
