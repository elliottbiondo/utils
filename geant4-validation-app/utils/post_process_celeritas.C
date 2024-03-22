//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file post_process_celeritas.C
//! \brief Post-process Celeritas MC Truth output data.
//---------------------------------------------------------------------------//
#include <iostream>
#include <map>
#include <string>
#include <TBranch.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TSystem.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <assert.h>

#include "../src/RootData.hh"

//---------------------------------------------------------------------------//
/*!
 * Using directives.
 */
using rootdata::process_map;
using rootdata::ProcessId;
using IdToTrackMap
    = std::map<decltype(rootdata::Track::parent_id), rootdata::Track>;

//---------------------------------------------------------------------------//
/*!
 * Map Action label and ProcessId.
 */
std::map<std::string, ProcessId> const action_processid_map = {
    // clang-format off
    // Physics actions
    {"ioni-moller-bhabha",        ProcessId::e_ioni},
    {"brems-sb",                  ProcessId::e_brems},
    {"brems-rel",                 ProcessId::e_brems},
    {"brems-combined",            ProcessId::e_brems},
    {"photoel-livermore",         ProcessId::photoelectric},
    {"scat-klein-nishina",        ProcessId::compton},
    {"conv-bethe-heitler",        ProcessId::conversion},
    {"scat-rayleigh",             ProcessId::rayleigh},
    {"annihil-2-gamma",           ProcessId::annihilation},
    {"coulomb-wentzel",           ProcessId::coulomb_scat},
    {"msc-range",                 ProcessId::msc_range},
    // Other actions
    {"pre-step",                  ProcessId::pre_step},
    {"eloss-range",               ProcessId::eloss_range},
    {"physics-discrete-select",   ProcessId::physics_discrete_select},
    {"physics-integral-rejected", ProcessId::physics_integral_rejected},
    {"physics-failure",           ProcessId::physics_failure},
    {"along-step-general-linear", ProcessId::along_step_general_linear},
    {"extend-from-primaries",     ProcessId::extend_from_primaries},
    {"initialize-tracks",         ProcessId::initialize_tracks},
    {"along-step-neutral",        ProcessId::along_step_neutral},
    {"geo-propagation-limit",     ProcessId::geo_propagation_limit},
    {"kill-looping",              ProcessId::kill_looping},
    {"geo-boundary",              ProcessId::geo_boundary},
    {"extend-from-secondaries",   ProcessId::extend_from_secondaries},
    {"action-diagnostic",         ProcessId::action_diagnostic},
    {"step-diagnostic",           ProcessId::step_diagnostic},
    {"step-gather-pre",           ProcessId::step_gather_pre},
    {"step-gather-post",          ProcessId::step_gather_post},
    {"not_mapped",                ProcessId::not_mapped}
    // clang-format on
};

//---------------------------------------------------------------------------//
/*!
 * Define unspecified track id.
 */
static constexpr auto unspecified
    = static_cast<decltype(rootdata::Track::parent_id)>(-1);

//---------------------------------------------------------------------------//
/*!
 * Helper function to tag primaries.
 */
bool is_primary(rootdata::Track const& track)
{
    return track.parent_id == unspecified;
}

//---------------------------------------------------------------------------//
/*!
 * Return a filled rootdata::Step object.
 */
rootdata::Step
store_step(TTree* steps_ttree, std::vector<std::string>* action_labels)
{
    rootdata::Step step;

    auto const action_id = steps_ttree->GetLeaf("action_id")->GetValue();
    auto const key = action_processid_map.find(action_labels->at(action_id));

    step.process_id = (key != action_processid_map.end())
                          ? key->second
                          : ProcessId::not_mapped;

    step.kinetic_energy = steps_ttree->GetLeaf("post_energy")->GetValue();
    step.global_time = steps_ttree->GetLeaf("post_time")->GetValue();
    step.length = steps_ttree->GetLeaf("step_length")->GetValue();

    auto const& post_dir = steps_ttree->GetLeaf("post_dir");
    step.direction = {
        post_dir->GetValue(0), post_dir->GetValue(1), post_dir->GetValue(2)};

    auto const& post_pos = steps_ttree->GetLeaf("post_pos");
    step.position = {
        post_pos->GetValue(0), post_pos->GetValue(1), post_pos->GetValue(2)};

    return step;
}

//---------------------------------------------------------------------------//
/*!
 * Store track ids and vertex information.
 */
void init_track(TTree* steps_ttree, rootdata::Track& track)
{
    track.id = steps_ttree->GetLeaf("track_id")->GetValue();
    track.pdg = steps_ttree->GetLeaf("particle")->GetValue();
    track.parent_id = steps_ttree->GetLeaf("parent_id")->GetValueLong64();
    track.vertex_energy = steps_ttree->GetLeaf("pre_energy")->GetValue();
    track.vertex_global_time = steps_ttree->GetLeaf("pre_time")->GetValue();

    auto const& pre_pos = steps_ttree->GetLeaf("pre_pos");
    auto const& pre_dir = steps_ttree->GetLeaf("pre_dir");
    track.vertex_position
        = {pre_pos->GetValue(0), pre_pos->GetValue(1), pre_pos->GetValue(2)};
    track.vertex_direction
        = {pre_dir->GetValue(0), pre_dir->GetValue(1), pre_dir->GetValue(2)};
}

//---------------------------------------------------------------------------//
/*!
 * Loop over the map<track_id, rootdata_track> and store primaries/secondaries
 * in a rootdata::Event accordingly.
 */
void store_event_tracks(rootdata::Event* event_ptr,
                        IdToTrackMap& trackid_track_map)
{
    // Loop over map and push_back primaries and secondaries
    for (auto& key : trackid_track_map)
    {
        // Sort steps by time
        std::sort(key.second.steps.begin(),
                  key.second.steps.end(),
                  [](rootdata::Step const& lhs, rootdata::Step const& rhs) {
                      return lhs.global_time < rhs.global_time;
                  });

        if (is_primary(key.second))
        {
            // Store primary
            event_ptr->primaries.push_back(key.second);
        }
        else
        {
            // Store secondary
            event_ptr->secondaries.push_back(key.second);
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Find data limits for this simulation run.
 */
void store_data_limits(rootdata::Event* event_ptr, rootdata::DataLimits* limits)
{
    limits->max_num_primaries
        = std::max(limits->max_num_primaries,
                   static_cast<std::size_t>(event_ptr->primaries.size()));

    limits->max_num_secondaries
        = std::max(limits->max_num_secondaries,
                   static_cast<std::size_t>(event_ptr->secondaries.size()));

    std::size_t steps_per_event{0};
    for (auto const& track : event_ptr->primaries)
    {
        limits->max_primary_num_steps
            = std::max(limits->max_primary_num_steps,
                       static_cast<std::size_t>(track.number_of_steps));
        limits->max_primary_energy
            = std::max(limits->max_primary_energy, track.vertex_energy);

        auto const& pos = track.vertex_position;
        limits->min_vertex = {std::min(limits->min_vertex.x, pos.x),
                              std::min(limits->min_vertex.y, pos.y),
                              std::min(limits->min_vertex.z, pos.z)};

        limits->max_vertex = {std::max(limits->max_vertex.x, pos.x),
                              std::max(limits->max_vertex.y, pos.y),
                              std::max(limits->max_vertex.z, pos.z)};

        steps_per_event += track.number_of_steps;
    }
    for (auto const& track : event_ptr->secondaries)
    {
        limits->max_secondary_num_steps
            = std::max(limits->max_secondary_num_steps,
                       static_cast<std::size_t>(track.number_of_steps));
        limits->max_secondary_energy
            = std::max(limits->max_secondary_energy, track.vertex_energy);

        auto const& pos = track.vertex_position;
        limits->min_vertex = {std::min(limits->min_vertex.x, pos.x),
                              std::min(limits->min_vertex.y, pos.y),
                              std::min(limits->min_vertex.z, pos.z)};

        limits->max_vertex = {std::max(limits->max_vertex.x, pos.x),
                              std::max(limits->max_vertex.y, pos.y),
                              std::max(limits->max_vertex.z, pos.z)};

        steps_per_event += track.number_of_steps;
    }

    limits->max_steps_per_event
        = std::max(limits->max_steps_per_event, steps_per_event);
}

//---------------------------------------------------------------------------//
/*!
 * Post-process a standard Celeritas ROOT MC truth output file, which only has
 * a flat structure where each TTree entry is a step from a given thread and
 * produce a ROOT file with rootdata::Event structs for easier analysis.
 */
void post_process_celeritas(std::string input_filename)
{
    // Load rootdata shared library
    gSystem->Load("../build/librootdata");

    // Load Celeritas MC truth input
    auto input = new TFile(input_filename.c_str(), "read");
    auto steps_tree = (TTree*)input->Get("steps");
    auto params_tree = (TTree*)input->Get("core_params");

    std::vector<std::string>* action_labels = nullptr;
    params_tree->SetBranchAddress("action_labels", &action_labels);
    params_tree->GetEntry(0);  // Load all labels at once

    // Sort tree by a major (event_id) and minor (track_id) value
    steps_tree->BuildIndex("event_id", "track_id");
    auto tree_index = (TTreeIndex*)steps_tree->GetTreeIndex();
    auto sorted_index = tree_index->GetIndex();

    // Create post-processed ROOT output
    std::string output_filename = input_filename;
    output_filename.resize(output_filename.size() - 5);
    output_filename += ".post.root";
    auto output = new TFile(output_filename.c_str(), "recreate");
    auto event_tree = new TTree("events", "events");
    auto data_limits_tree = new TTree("limits", "limits");

    // Initialize branch with a rootdata::Event object
    rootdata::Event* event_ptr = nullptr;
    event_tree->Branch("event", &event_ptr);

    // Initialize branch with a rootdata::DataLimits object
    rootdata::DataLimits* data_limits_ptr = nullptr;
    data_limits_tree->Branch("data_limits", &data_limits_ptr);
    // Initialize data limits values
    data_limits_ptr = new rootdata::DataLimits();

    // Get first event id
    steps_tree->GetEntry(sorted_index[0]);
    auto last_evtid = steps_tree->GetLeaf("event_id")->GetValue();

    IdToTrackMap trkid_track_map;

    // Set up progress indicator
    float const percent_increment = 1;  // Print msg at every increment [%]
    int const events_per_print = (percent_increment / 100)
                                 * steps_tree->GetEntries();
    int events_per_print_counter = 0;  // Addition is better than modulo
    int n_printed_msgs = 0;  // One printed msg for every percent increment
    std::cout << "Processing " << input_filename << ": 0%";
    std::cout.flush();

    // Loop over input data and fill event_ptr
    for (int i = 0; i < steps_tree->GetEntries(); i++)
    {
        steps_tree->GetEntry(sorted_index[i]);
        int event_id = steps_tree->GetLeaf("event_id")->GetValue();

        if (++events_per_print_counter == events_per_print)
        {
            // Reached the number of events per print
            // Increment number of printed messages and reset counter
            n_printed_msgs++;
            events_per_print_counter = 0;
            std::cout << "\rProcessing " << input_filename << ": "
                      << n_printed_msgs * percent_increment << "%";
            std::cout.flush();
        }

        if (last_evtid != event_id)
        {
            // Found new event (never happens at i == 0)
            // Loop over map and fill primaries and secondaries
            store_event_tracks(event_ptr, trkid_track_map);
            // Find and store data limits
            store_data_limits(event_ptr, data_limits_ptr);

            // Fill tree and prepare for next event
            event_tree->Fill();
            event_ptr = new rootdata::Event();
            trkid_track_map.clear();
            last_evtid = event_id;
        }

        // Store event data
        event_ptr->id = event_id;
        int const track_id = steps_tree->GetLeaf("track_id")->GetValue();

        if (trkid_track_map.find(track_id) == trkid_track_map.end())
        {
            // Track id does not exist; create new map track slot
            rootdata::Track track;

            // Note: In rare cases the first step is not the vertex
            init_track(steps_tree, track);
            trkid_track_map.insert({track_id, std::move(track)});
        }

        // Locate track_id
        auto key = trkid_track_map.find(track_id);

        if (steps_tree->GetLeaf("pre_time")->GetValue()
            < key->second.vertex_global_time)
        {
            // Current step time is smaller than the existing track
            // initialization values and therefore happened *before*.
            // Override initial values until the actual track vertex is found.
            init_track(steps_tree, key->second);
        }

        // Increment track information and store step data
        key->second.length += steps_tree->GetLeaf("step_length")->GetValue();
        key->second.energy_dep
            += steps_tree->GetLeaf("energy_deposition")->GetValue();
        key->second.steps.push_back(store_step(steps_tree, action_labels));
        key->second.number_of_steps++;
    }

    std::cout << std::endl;
    std::cout.flush();

    // Write last event to the tree and store data limits
    store_event_tracks(event_ptr, trkid_track_map);
    store_data_limits(event_ptr, data_limits_ptr);

    // Fill and write trees; close ROOT files
    event_tree->Fill();
    data_limits_tree->Fill();
    output->Write();
    output->Close();
    input->Close();
}
