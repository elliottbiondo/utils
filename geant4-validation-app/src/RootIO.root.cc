//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RootIO.cc
//---------------------------------------------------------------------------//
#include "RootIO.hh"

#include <iostream>
#include <G4RunManager.hh>
#include <TFile.h>
#include <TTree.h>
#include <assert.h>

#include "HepMC3Reader.hh"
#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Singleton declaration.
 */
static RootIO* rootio_singleton = nullptr;

//---------------------------------------------------------------------------//
// PUBLIC
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Constructor singleton.
 */
void RootIO::construct(char const* root_filename)
{
    if (!rootio_singleton)
    {
        assert(root_filename);
        rootio_singleton = new RootIO(root_filename);
    }
    else
    {
        std::cout << "ROOT I/O already constructed. Nothing to do.\n";
    }
}

//---------------------------------------------------------------------------//
/*!
 * Get static RootIO instance. \c construct() *MUST* be called before this.
 */
RootIO* RootIO::instance()
{
    assert(rootio_singleton);
    return rootio_singleton;
}

//---------------------------------------------------------------------------//
/*!
 * Clear event_ struct.
 */
void RootIO::clear_event()
{
    event_ = rootdata::Event();
    event_.sensitive_detectors.resize(sdgdml_sensdetidx_.size());
}

//---------------------------------------------------------------------------//
/*!
 * Clear track_ struct.
 */
void RootIO::clear_track()
{
    track_ = rootdata::Track();
}

//---------------------------------------------------------------------------//
/*!
 * Add new sensitive detector to the map. This maps {name, copy_number} to a
 * global index in \c event.sensitive_detectors .
 */
void RootIO::add_sd(rootdata::SensDetGdml from_gdml)
{
    // Assert that sd_name was not already added to the map
    assert(sdgdml_sensdetidx_.find(from_gdml) == sdgdml_sensdetidx_.end());

    // Map sensitive detector name/copy number with its new global id
    auto sd_index = sdgdml_sensdetidx_.size();
    sdgdml_sensdetidx_.insert({from_gdml, sd_index});
}

//---------------------------------------------------------------------------//
/*!
 * Fill event TTree.
 */
void RootIO::fill_event_ttree()
{
    ttree_event_->Fill();
}

//---------------------------------------------------------------------------//
/*!
 * Fill data limits TTree.
 */
void RootIO::fill_data_limits_ttree()
{
    ttree_data_limits_->Fill();
}

//---------------------------------------------------------------------------//
/*!
 * Store performance metrics information.
 */
void RootIO::store_performance_metrics(rootdata::ExecutionTime& exec_times)
{
    std::unique_ptr<TTree> ttree_performance;
    ttree_performance.reset(new TTree("performance", "performance"));
    ttree_performance->Branch("execution_times", &exec_times);
    ttree_performance->Fill();
    ttree_performance->Write();
}

//---------------------------------------------------------------------------//
/*!
 * Store map containing sensitive detector names and ids.
 */
void RootIO::store_sd_map()
{
    std::unique_ptr<TTree> ttree_sd_map;
    ttree_sd_map.reset(new TTree("sensitive_detectors", "sensitive_detectors"));

    std::string name;
    unsigned int copy_num;
    unsigned int event_sd_index;
    ttree_sd_map->Branch("name", &name);
    ttree_sd_map->Branch("copy_num", &copy_num);
    ttree_sd_map->Branch("event_sd_index", &event_sd_index);

    for (auto const& key : sdgdml_sensdetidx_)
    {
        name = key.first.name;
        copy_num = key.first.copy_number;
        event_sd_index = key.second;
        ttree_sd_map->Fill();
    }

    ttree_sd_map->Write();
}

//---------------------------------------------------------------------------//
/*!
 * Store json input information in the ROOT file for future reference.
 */
void RootIO::store_input()
{
    assert(tfile_->IsOpen());

    // >>> Fetch input data
    auto const& json = JsonReader::instance()->json();
    auto* hepmc3_reader = HepMC3Reader::instance();

    unsigned int g4_version = G4VERSION_NUMBER;

    // Geometry
    std::string geometry_name = json.at("geometry").get<std::string>();

    // Simulation
    auto const& json_sim = json.at("simulation");
    std::string hepmc3_inp = json_sim.at("hepmc3").get<std::string>();
    auto const json_gun = json_sim.at("particle_gun");

    std::string simulation = !hepmc3_inp.empty() ? hepmc3_inp : "particle_gun";
    std::size_t events = !hepmc3_inp.empty()
                             ? hepmc3_reader->number_of_events()
                             : json_gun.at("events").get<size_t>();

    int pdg = json_gun.at("pdg").get<int>();
    double energy = json_gun.at("energy").get<double>();

    double vertex[3], direction[3];
    for (int i = 0; i < 3; i++)
    {
        vertex[i] = json_gun.at("vertex")[i].get<double>();
        direction[i] = json_gun.at("direction")[i].get<double>();
    }

    long seed = CLHEP::HepRandom::getTheSeed();
    std::string rng = CLHEP::HepRandom::getTheEngine()->name();
    int threads = USE_MT ? json_sim.at("threads").get<int>() : 1;
    bool spline = json_sim.at("spline").get<bool>();
    bool eloss_fluct = json_sim.at("eloss_fluctuation").get<bool>();

    // Physics list
    auto const jphys = json.at("physics");

    bool compton_scattering = jphys.at("compton_scattering").get<bool>();
    bool photoelectric = jphys.at("photoelectric").get<bool>();
    bool rayleigh_scattering = jphys.at("rayleigh_scattering").get<bool>();
    bool gamma_conversion = jphys.at("gamma_conversion").get<bool>();
    bool positron_annihilation = jphys.at("positron_annihilation").get<bool>();
    bool bremsstrahlung = jphys.at("bremsstrahlung").get<bool>();
    bool e_ionization = jphys.at("e_ionization").get<bool>();
    bool coulomb_scattering = jphys.at("coulomb_scattering").get<bool>();
    bool msc_low = jphys.at("multiple_scattering_low").get<bool>();
    bool msc_high = jphys.at("multiple_scattering_high").get<bool>();
    bool scint = jphys.at("scintillation").get<bool>();
    bool cerenkov = jphys.at("cerenkov").get<bool>();

    // >>> Store info into branches
    std::unique_ptr<TTree> ttree_input;
    ttree_input.reset(new TTree("input", "input"));

    ttree_input->Branch("version", &g4_version);
    ttree_input->Branch("geometry", &geometry_name);
    ttree_input->Branch("simulation", &simulation);
    ttree_input->Branch("events", &events);

    if (hepmc3_inp.empty())
    {
        // No HepMC3 input; store particle gun info
        ttree_input->Branch("pdg", &pdg);
        ttree_input->Branch("energy", &energy);
        ttree_input->Branch("vertex", vertex, "vertex[3]/D");
        ttree_input->Branch("directions", direction, "direction[3]/D");
    }

    ttree_input->Branch("threads", &threads);
    ttree_input->Branch("seed", &seed);
    ttree_input->Branch("rng", &rng);
    ttree_input->Branch("spline", &spline);
    ttree_input->Branch("eloss_fluctuation", &eloss_fluct);

    ttree_input->Branch("compton_scattering", &compton_scattering);
    ttree_input->Branch("photoelectric", &photoelectric);
    ttree_input->Branch("rayleigh_scattering", &rayleigh_scattering);
    ttree_input->Branch("gamma_conversion", &gamma_conversion);
    ttree_input->Branch("positron_annihilation", &positron_annihilation);
    ttree_input->Branch("bremsstrahlung", &bremsstrahlung);
    ttree_input->Branch("e_ionization", &e_ionization);
    ttree_input->Branch("coulomb_scattering", &coulomb_scattering);
    ttree_input->Branch("multiple_scattering_low", &msc_low);
    ttree_input->Branch("multiple_scattering_high", &msc_high);
    ttree_input->Branch("scintillation", &scint);
    ttree_input->Branch("cerenkov", &cerenkov);

    ttree_input->Fill();
    ttree_input->Write();
}

//---------------------------------------------------------------------------//
/*!
 * Return performance run bool.
 */
bool RootIO::is_performance_run()
{
    return is_performance_run_;
}

//---------------------------------------------------------------------------//
/*!
 * Write TFile.
 */
void RootIO::write_tfile()
{
    tfile_->Write();
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Construct new TFile with ROOT filename.
 */
RootIO::RootIO(char const* root_filename)
{
    tfile_.reset(TFile::Open(root_filename, "recreate"));

    ttree_event_.reset(new TTree("events", "events"));
    ttree_event_->Branch("event", &event_);

    ttree_data_limits_.reset(new TTree("limits", "limits"));
    ttree_data_limits_->Branch("data_limits", &data_limits_);
    data_limits_ = rootdata::DataLimits();

    auto const json = JsonReader::instance()->json();
    is_performance_run_
        = json.at("simulation").at("performance_run").get<bool>();

    if (USE_MT && !is_performance_run_)
    {
        is_performance_run_ = true;

        std::cout << "WARNING: Cannot store full MC truth information with "
                     "USE_MT=ON, as ROOT I/O is not thread-safe. Input and "
                     "performance values can be stored at the end of the run."
                  << std::endl;
    }
}
