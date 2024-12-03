//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file ValidationGlobals.hh
//! \brief Global definitions for \c validation.C
//---------------------------------------------------------------------------//
/*!
 * \note
 * This set of global variables and helper functions should make the main macro
 * easier to read and update. Classes were avoided so that we could simply run
 * the \c validation.C macro without the need to rebuild dictionaries for every
 * change, as well as load them into ROOT every time we wanted to run the
 * macro.
 */
//---------------------------------------------------------------------------//
#pragma once

#include <TDatabasePDG.h>

#include "../src/RootData.hh"

class TH1F;
class TH2F;
class TGraph;

//---------------------------------------------------------------------------//
/*!
 * The namespace \c vg (validation globals) stores any data that needs to
 * be accessed by all functions, including the main macro.
 */
namespace vg
{
//---------------------------------------------------------------------------//
/*!
 * Set up miscellaneous global variables.
 */

// Count total steps in the simulation for performance metrics
static long total_num_events = 0;
static double total_num_steps = 0;

// Total steps per event (for vg::hist.n_steps_evt)
static double num_steps_per_event = 0;

// Set up maps for scaling histograms
std::map<int, std::string> particle_map;
std::map<rootdata::ProcessId, std::string> process_map;
TDatabasePDG pdg_db;

// Extend plot margins. Limits start at 1. Thus 1.1 adds 10%
static double const plot_margin = 1.1;

// Canvas margins
static double const bottom_margin = 0.15;
static double const right_margin = 0.15;
static double const left_margin = 0.13;
static double const label_size = 0.06;
static double const label_offset = 0.015;
static double const marker_size = 1.5;
static double const marker_color = 15;

// Select plot data safely
enum MC
{
    G4,
    Cel
};

//---------------------------------------------------------------------------//
/*!
 * Store TH1 and TH2 pointers.
 */
struct HistData
{
    int step_bins = 30;
    int energy_bins = 30;

    int vertex_bins = 100;
    double vtx_min;
    double vtx_max;

    int sd_edep_bins = 30;
    int sd_steps_bins = 100;

    // Array is for MC::G4 and MC::Cel
    TH1D* n_steps_prim;
    TH1D* n_secondaries;
    TH1D* n_steps_sec;
    TH1D* n_steps_evt;
    TH1D* steps_process;
    TH2D* particle_process;

    TH1D* prim_edep;
    TH1D* prim_step_edep;
    TH1D* sec_edep;
    TH1D* sec_step_edep;
    TH1D* sec_energy;

    TH1D* sec_vtx_x;
    TH1D* sec_vtx_y;
    TH1D* sec_vtx_z;
    TH1D* sec_vtx_r;
    TH1D* sec_vtx_theta;

    TH1D* sec_dir_x;
    TH1D* sec_dir_y;
    TH1D* sec_dir_z;

    TH1D* step_prim_time;
    TH1D* step_sec_time;
    TH1D* vtx_prim_time;
    TH1D* vtx_sec_time;

    TH1D* prim_length;
    TH1D* sec_length;
    TH1D* prim_step_length;
    TH1D* sec_step_length;

    TH1D* sitracker_edep;
    TH1D* emcalo_edep;
    TH1D* sitracker_nsteps;
    TH1D* emcalo_nsteps;
};

// Array is for MC::G4 and MC::Cel
// Ptrs are initialized in ValidationGlobalsHelper.hh::initialize_histograms()
static HistData histograms[2];

//---------------------------------------------------------------------------//
/*!
 * Store TGraph pointers.
 */
struct GraphData
{
    // Define binning
    int const r_n_bins = 1025;  // Select the number of bins
    double r_bin_min = 0;
    double r_bin_max = 700;
    double r_bin_size = r_bin_max / r_n_bins;
    double r_bin_half_width = r_bin_size / 2;

    int const z_n_bins = 50;  // Select the number of bins
    double z_bin_min = -20;
    double z_bin_max = 20;
    double z_bin_size = (z_bin_max - z_bin_min) / z_n_bins;
    double z_bin_half_width = z_bin_size / 2;

    struct Bin
    {
        double value;
        double min;
        double center;
        double max;
    };

    std::vector<GraphData::Bin> r_bins;
    std::vector<GraphData::Bin> z_bins;

    TGraph* cumulative_r = new TGraph();
    TGraph* cumulative_z = new TGraph();
};

// Array is for MC::G4 and MC::Cel
// Binning is set in ValidationGlobalsHelper.hh::initialize_cumulative_bins()
static GraphData graphs[2];

//---------------------------------------------------------------------------//
/*!
 * Store global data limits and performance metrics when many files are read.
 */
static rootdata::DataLimits data_limits;
static rootdata::ExecutionTime exec_time;

//---------------------------------------------------------------------------//
}  // namespace vg

//---------------------------------------------------------------------------//
// Helper functions for the vg namespace data
//---------------------------------------------------------------------------//
#include "ValidationGlobalsHelper.hh"
