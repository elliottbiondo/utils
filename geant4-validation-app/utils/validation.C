//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file validation.C
//! \brief Macro with a set of validation plots.
//---------------------------------------------------------------------------//
#include <algorithm>
#include <fstream>
#include <TSystem.h>

#include "ValidationHelper.hh"

using std::cout;
using std::endl;
using std::max;

//---------------------------------------------------------------------------//
/*!
 * Helper functions declaration. Definitions are below the main macro.
 */
void read_single_file(TFile* input, vg::MC mc_enum);
void read_file_list(std::ifstream& input_list);
void print_performance();

//---------------------------------------------------------------------------//
// MAIN MACRO
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * ROOT Geant4 vs. Celeritas validation macro.
 *
 * Input can be either a root or a text file listing all ROOT files to be read.
 *
 * Usage:
 * root[0] .x validation.C("g4_output.root", "cel_output.root")
 * or
 * root[0] .x validation.C("g4_file_list.txt", "cel_file_list.txt")
 *
 * The second input can be empty for checking plots from a single input:
 * root[0] .x validation.C("mctruth.root", "")
 * root[0] .x validation.C("mctruth-list.txt", "")
 */
void validation(std::string arg_g4, std::string arg_cel)
{
    // Load rootdata shared library
    char const* librootdata = "../build/librootdata";
    gSystem->Load(librootdata);

    // Check if file extension is a .root
    bool is_root
        = (arg_g4.length() > 4 && arg_g4.substr(arg_g4.length() - 4) == "root")
              ? true
              : false;

    if (is_root)
    {
        // >>> Single ROOT file provided
        TFile* input_g4 = TFile::Open(arg_g4.c_str(), "read");
        TTree* limits_tree = (TTree*)input_g4->Get("limits");
        rootdata::DataLimits* data_limits = nullptr;
        limits_tree->SetBranchAddress("data_limits", &data_limits);
        limits_tree->GetEntry(0);

        // Initialize global histograms and graphs
        vg::initialize_histograms(*data_limits);
        vg::initialize_cumulative_bins();

        // Read first input
        read_single_file(input_g4, vg::MC::G4);

        if (!arg_cel.empty())
        {
            // Read second input
            // TODO: make sure second input is a root file too
            TFile* input_cel = TFile::Open(arg_cel.c_str(), "read");
            read_single_file(input_cel, vg::MC::Cel);
        }
    }

    else
    {
        // >>> List of ROOT files provided
        std::ifstream input_list(arg_g4);
        if (!input_list.is_open())
        {
            cout << "Could not open " << arg_g4 << endl;
            return;
        }

        // TODO: Update with 2 lists for comparison
        read_file_list(input_list);
    }

    // Create canvases
    draw_canvas_step();
    draw_canvas_energy();
    draw_canvas_vertex();
    draw_canvas_time();
    draw_canvas_length();
    // draw_canvas_cumulative();
    // draw_canvas_sensitive_detectors();
    // print_performance();
}

//---------------------------------------------------------------------------//
// HELPER FUNCTIONS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Read single root output file.
 * \c vg::hist and \c vg::graph *MUST* be initialized before this call.
 */
void read_single_file(TFile* input, vg::MC mc_enum)
{
    // Loop over data and fill vg histograms and graphs
    loop(input, mc_enum);

    // Add to global performance metrics
    TTree* event_tree = (TTree*)input->Get("events");
    TTree* performance_tree = (TTree*)input->Get("performance");

    if (performance_tree)
    {
        rootdata::ExecutionTime* exec_times = nullptr;
        performance_tree->SetBranchAddress("execution_times", &exec_times);
        performance_tree->GetEntry(0);

        vg::exec_time.wall_total += exec_times->wall_total;
        vg::exec_time.wall_sim_run += exec_times->wall_sim_run;
        vg::exec_time.cpu_total += exec_times->cpu_total;
        vg::exec_time.cpu_sim_run += exec_times->cpu_sim_run;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Read text file listing all ROOT files. The list must contain one file per
 * line.
 */
void read_file_list(std::ifstream& input_list)
{
    auto& vg_limits = vg::data_limits;
    std::string line;

    while (std::getline(input_list, line))
    {
        TFile* input = new TFile(line.c_str(), "read");

        // Collect correct global data limits
        TTree* limits_tree = (TTree*)input->Get("limits");
        rootdata::DataLimits* limits = nullptr;
        limits_tree->SetBranchAddress("data_limits", &limits);
        limits_tree->GetEntry(0);

        vg_limits.max_num_primaries
            = max(vg_limits.max_num_primaries, limits->max_num_primaries);

        vg_limits.max_num_secondaries
            = max(vg_limits.max_num_secondaries, limits->max_num_secondaries);

        vg_limits.max_primary_energy
            = max(vg_limits.max_primary_energy, limits->max_primary_energy);

        vg_limits.max_primary_num_steps = max(vg_limits.max_primary_num_steps,
                                              limits->max_primary_num_steps);

        vg_limits.max_sd_energy
            = max(vg_limits.max_sd_energy, limits->max_sd_energy);

        vg_limits.max_sd_num_steps
            = max(vg_limits.max_sd_num_steps, limits->max_sd_num_steps);

        vg_limits.max_secondary_energy = max(vg_limits.max_secondary_energy,
                                             limits->max_secondary_energy);

        vg_limits.max_secondary_num_steps = max(
            vg_limits.max_secondary_num_steps, limits->max_secondary_num_steps);

        vg_limits.max_steps_per_event
            = max(vg_limits.max_steps_per_event, limits->max_steps_per_event);

        vg_limits.max_vertex.x
            = max(vg_limits.max_vertex.x, limits->max_vertex.x);
        vg_limits.max_vertex.y
            = max(vg_limits.max_vertex.y, limits->max_vertex.y);
        vg_limits.max_vertex.z
            = max(vg_limits.max_vertex.z, limits->max_vertex.z);

        vg_limits.min_vertex.x
            = max(vg_limits.min_vertex.x, limits->min_vertex.x);
        vg_limits.min_vertex.y
            = max(vg_limits.min_vertex.y, limits->min_vertex.y);
        vg_limits.min_vertex.z
            = max(vg_limits.min_vertex.z, limits->min_vertex.z);
    }

    // Rewind back to the begining of file
    input_list.clear();
    input_list.seekg(0, std::ios::beg);

    // Initialize global histograms and graphs
    vg::initialize_histograms(vg_limits);
    vg::initialize_cumulative_bins();

    while (std::getline(input_list, line))
    {
        // Loop over files and fill histograms
        read_single_file(new TFile(line.c_str(), "read"), vg::MC::G4);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Print performance metrics.
 */
void print_performance()
{
    using vg::exec_time;
    using vg::total_num_events;
    using vg::total_num_steps;

    double init_time = exec_time.cpu_total - exec_time.cpu_sim_run;
    double time_per_event = exec_time.cpu_sim_run / total_num_events;
    double time_per_step = exec_time.cpu_sim_run / total_num_steps;

    cout << endl;
    cout << std::fixed << std::scientific;
    cout << "| Performance metric | Time [s]     |" << endl;
    cout << "| ------------------ | ------------ |" << endl;
    cout << "| Wall total         | " << exec_time.wall_total << " |" << endl;
    cout << "| CPU total          | " << exec_time.cpu_total << " |" << endl;
    cout << "| Initialization     | " << init_time << " |" << endl;
    cout << "| Per event          | " << time_per_event << " |" << endl;
    cout << "| Per step           | " << time_per_step << " |" << endl;
    cout << endl;
}
