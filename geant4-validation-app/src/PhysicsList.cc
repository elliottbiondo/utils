//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.cc
//---------------------------------------------------------------------------//
#include "PhysicsList.hh"

#include <G4AntiNeutrinoE.hh>
#include <G4AntiNeutrinoMu.hh>
#include <G4Cerenkov.hh>
#include <G4ComptonScattering.hh>
#include <G4CoulombScattering.hh>
#include <G4Decay.hh>
#include <G4Electron.hh>
#include <G4Gamma.hh>
#include <G4GammaConversion.hh>
#include <G4GenericIon.hh>
#include <G4LivermorePhotoElectricModel.hh>
#include <G4LivermoreRayleighModel.hh>
#include <G4MollerBhabhaModel.hh>
#include <G4MuonMinus.hh>
#include <G4MuonPlus.hh>
#include <G4NeutrinoE.hh>
#include <G4NeutrinoMu.hh>
#include <G4OpRayleigh.hh>
#include <G4OpticalParameters.hh>
#include <G4OpticalPhoton.hh>
#include <G4PairProductionRelModel.hh>
#include <G4PhotoElectricEffect.hh>
#include <G4PhysicsListHelper.hh>
#include <G4Positron.hh>
#include <G4ProcessManager.hh>
#include <G4Proton.hh>
#include <G4RayleighScattering.hh>
#include <G4Scintillation.hh>
#include <G4SystemOfUnits.hh>
#include <G4TransportationManager.hh>
#include <G4UrbanMscModel.hh>
#include <G4WentzelVIModel.hh>
#include <G4eCoulombScatteringModel.hh>
#include <G4eIonisation.hh>
#include <G4eMultipleScattering.hh>
#include <G4eeToTwoGammaModel.hh>
#include <G4eplusAnnihilation.hh>

#include "BremsstrahlungProcess.hh"
#include "G4appMacros.hh"
#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct, load list of physics processes, and set verbosity.
 */
PhysicsList::PhysicsList() : G4VUserPhysicsList()
{
    auto const json = JsonReader::instance()->json();
    auto const& json_sim = json.at("simulation");

    // Update selected processes according to the json
    for (auto pair : selected_processes_)
    {
        selected_processes_.at(pair.first)
            = json.at("physics").at(pair.first).get<bool>();
    }

    auto em_parameters = G4EmParameters::Instance();
    bool eloss_fluct = json_sim.at("eloss_fluctuation").get<bool>();
    em_parameters->SetLossFluctuations(eloss_fluct);
    int level = json.at("verbosity").at("PhysicsList").get<int>();
    em_parameters->SetVerbose(level);

    optical_ = (selected_processes_.find("scintillation")->second
                || selected_processes_.find("cerenkov")->second
                || selected_processes_.find("optical_rayleigh")->second);

    decay_ = selected_processes_.find("muon_decay")->second;

    // TODO implement binning
    // em_parameters->SetNumberOfBinsPerDecade(nbins);

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

    bool const msc
        = (selected_processes_.find("multiple_scattering_low")->second
           || selected_processes_.find("multiple_scattering_high")->second);
    bool const coulomb_on
        = selected_processes_.find("coulomb_scattering")->second;

    if (msc || coulomb)
    {
        G4GenericIon::GenericIonDefinition();
    }

    if (optical_)
    {
        G4OpticalPhoton::OpticalPhotonDefinition();
    }

    if (decay_)
    {
        G4MuonMinus::MuonMinusDefinition();
        G4MuonPlus::MuonPlusDefinition();
        G4NeutrinoE::NeutrinoEDefinition();
        G4AntiNeutrinoE::AntiNeutrinoE();
        G4NeutrinoMu::NeutrinoMuDefinition();
        G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();
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

    // Add EM and optical processes
    this->add_gamma_processes();
    this->add_e_processes(G4Electron::Electron());
    this->add_e_processes(G4Positron::Positron());
    if (optical_)
    {
        this->add_optical_processes();
    }
    if (decay_)
    {
        this->add_decay_processes(G4MuonMinus::MuonMinus());
        this->add_decay_processes(G4MuonPlus::MuonPlus());
    }
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Add EM processes for photons.
 *
 * | Process              | Model class                   |
 * | -------------------- | ----------------------------- |
 * | Compton scattering   | G4KleinNishinaCompton         |
 * | Photoelectric effect | G4LivermorePhotoElectricModel |
 * | Rayleigh scattering  | G4LivermoreRayleighModel      |
 * | Gamma conversion     | G4PairProductionRelModel      |
 */
void PhysicsList::add_gamma_processes()
{
    auto* physics_list = G4PhysicsListHelper::GetPhysicsListHelper();
    auto const gamma = G4Gamma::Gamma();

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
 * | Process                      | Model class               |
 * | ---------------------------- | ------------------------- |
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
    auto* physics_list = G4PhysicsListHelper::GetPhysicsListHelper();

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
        auto models = BremsstrahlungProcess::ModelSelection::all;
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
            auto* bremsstrahlung = dynamic_cast<BremsstrahlungProcess*>(
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
        double const msc_threshold_energy
            = G4EmParameters::Instance()->MscEnergyLimit();

        auto coulomb_process = std::make_unique<G4CoulombScattering>();
        auto coulomb_model = std::make_unique<G4eCoulombScatteringModel>();
        coulomb_process->SetMinKinEnergy(msc_threshold_energy);
        coulomb_model->SetLowEnergyLimit(msc_threshold_energy);
        coulomb_model->SetActivationLowEnergyLimit(msc_threshold_energy);
        coulomb_process->SetEmModel(coulomb_model.release());
        physics_list->RegisterProcess(coulomb_process.release(), particle);
    }

    bool const msc_low
        = selected_processes_.find("multiple_scattering_low")->second;
    bool const msc_high
        = selected_processes_.find("multiple_scattering_high")->second;

    if (msc_low || msc_high)
    {
        // Multiple scattering
        double const msc_threshold_energy
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

//---------------------------------------------------------------------------//
/*!
 * Add optical physics processes to all applicable particles.
 *
 * | Process          | Model class      |
 * | ---------------- | ---------------- |
 * | Scintillation    | G4Scintillation  |
 * | Cerenkov         | G4Cerenkov       |
 * | Optical Rayleigh | G4OpRayleigh     |
 */
void PhysicsList::add_optical_processes()
{
    using G4PVDII = G4ProcessVectorDoItIndex;

    auto const* params = G4OpticalParameters::Instance();

    if (selected_processes_.find("optical_rayleigh")->second)
    {
        // G4OpRayleigh
        auto* opt_gamma_mgr
            = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
        assert(opt_gamma_mgr);

        if (params->GetProcessActivation("OpRayleigh"))
        {
            opt_gamma_mgr->AddDiscreteProcess(new G4OpRayleigh());
        }
    }

    auto* cerenkov = new G4Cerenkov();
    auto* scintillation = new G4Scintillation();

    // Ensure material-only scintillation until implemented in Celeritas
    scintillation->SetScintillationByParticleType(false);

    // Assign scintillation and Cerenkov to all applicable particles
    auto iter_part = GetParticleIterator();
    iter_part->reset();
    while ((*iter_part)())
    {
        auto const& particle_def = *iter_part->value();
        assert(&particle_def);
        auto const& particle_name = particle_def.GetParticleName();
        auto* proc_mgr = particle_def.GetProcessManager();
        assert(proc_mgr);

        if (selected_processes_.find("scintillation")->second)
        {
            if (scintillation->IsApplicable(particle_def)
                && params->GetProcessActivation("Scintillation"))
            {
                // G4Scintillation
                proc_mgr->AddProcess(scintillation);
                proc_mgr->SetProcessOrderingToLast(scintillation,
                                                   G4PVDII::idxAtRest);
                proc_mgr->SetProcessOrderingToLast(scintillation,
                                                   G4PVDII::idxPostStep);
            }
        }

        if (selected_processes_.find("cerenkov")->second)
        {
            if (cerenkov->IsApplicable(particle_def)
                && params->GetProcessActivation("Cerenkov"))
            {
                // G4Cerenkov
                proc_mgr->AddProcess(cerenkov);
                proc_mgr->SetProcessOrdering(cerenkov, G4PVDII::idxPostStep);
            }
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Add decay processes to all applicable particles.
 *
 * | Process          | Class               |
 * | ---------------- | ------------------- |
 * | Muon decay       | G4MuonDecayChannel  |
 */
void PhysicsList::add_decay_processes(G4ParticleDefinition* particle)
{
    auto* physics_list = G4PhysicsListHelper::GetPhysicsListHelper();
    if (selected_processes_.find("muon_decay")->second)
    {
        physics_list->RegisterProcess(new G4Decay(), particle);
    }
}
