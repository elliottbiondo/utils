//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file ValidationGlobalsHelper.hh
//! \brief Helper functions for validation globals (vg) namespace data
//---------------------------------------------------------------------------//
#include <TH1D.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TMath.h>
#include <Math/Vector3D.h>

#include "../src/RootData.hh"

namespace vg
{
//---------------------------------------------------------------------------//
/*!
 * Initialize TH1 and TH2 pointers.
 */
void initialize_histograms(const rootdata::DataLimits& data_limits)
{
    // Initialize histograms for both G4 and Celeritas
    for (int i = 0; i < 2; i++)
    {
        auto& hist = vg::histograms[i];

        hist.n_steps_prim
            = new TH1D("",
                       "Number of steps per primary",
                       hist.step_bins,
                       0,
                       data_limits.max_primary_num_steps * plot_margin);

        hist.n_secondaries
            = new TH1D("",
                       "Number of secondaries per event",
                       hist.step_bins,
                       0,
                       data_limits.max_num_secondaries * plot_margin);

        hist.n_steps_sec
            = new TH1D("",
                       "Number of steps per secondary",
                       hist.step_bins,
                       0,
                       data_limits.max_secondary_num_steps * plot_margin);

        hist.n_steps_evt
            = new TH1D("",
                       "Number of steps per event",
                       hist.step_bins,
                       0,
                       data_limits.max_steps_per_event * plot_margin);

        hist.steps_process    = new TH1D("", "Steps per process", 1, 0, 1);
        hist.particle_process = new TH2D(
            "", "Fraction of steps per process per particle", 1, 0, 1, 1, 0, 1);

        // Energy distributions
        hist.prim_edep = new TH1D("",
                                  "Primary energy deposition",
                                  hist.energy_bins,
                                  0,
                                  data_limits.max_primary_energy * plot_margin);

        hist.prim_step_edep = new TH1D("",
                                       "Primary step energy deposition",
                                       hist.energy_bins,
                                       0,
                                       data_limits.max_sd_energy * plot_margin);

        hist.sec_edep
            = new TH1D("",
                       "Secondary energy deposition",
                       hist.energy_bins,
                       0,
                       data_limits.max_secondary_energy * plot_margin);

        hist.sec_step_edep = new TH1D("",
                                      "Secondary step energy deposition",
                                      hist.energy_bins,
                                      0,
                                      data_limits.max_sd_energy * plot_margin);

        hist.sec_energy
            = new TH1D("",
                       "Secondary vertex energy",
                       hist.energy_bins,
                       0,
                       data_limits.max_secondary_energy * plot_margin);

        // Vertex distributions
        int  vertex_bins = 50;
        auto vtx_min     = data_limits.min_vertex;
        auto vtx_max     = data_limits.max_vertex;

        hist.sec_vtx_x = new TH1D("",
                                  "Secondary vertex x",
                                  vertex_bins,
                                  vtx_min.x * plot_margin,
                                  vtx_max.x * plot_margin);
        hist.sec_vtx_y = new TH1D("",
                                  "Secondary vertex y",
                                  vertex_bins,
                                  vtx_min.y * plot_margin,
                                  vtx_max.y * plot_margin);
        hist.sec_vtx_z = new TH1D("",
                                  "Secondary vertex z",
                                  vertex_bins,
                                  vtx_min.z * plot_margin,
                                  vtx_max.z * plot_margin);

        ROOT::Math::XYZVector xyz_vec(vtx_max.x, vtx_max.y, vtx_max.z);

        hist.sec_vtx_r = new TH1D("",
                                  "Secondary vertex r",
                                  vertex_bins,
                                  0,
                                  xyz_vec.rho() * plot_margin);

        hist.sec_vtx_theta
            = new TH1D("", "Secondary vertex #theta", 100, 0, TMath::Pi());

        // Vertex directions
        int    dir_bins = 50;
        double dir_min  = -1;
        double dir_max  = 1;

        hist.sec_dir_x = new TH1D(
            "", "Secondary vertex direction x", dir_bins, dir_min, dir_max);
        hist.sec_dir_y = new TH1D(
            "", "Secondary vertex direction y", dir_bins, dir_min, dir_max);
        hist.sec_dir_z = new TH1D(
            "", "Secondary vertex direction z", dir_bins, dir_min, dir_max);

        // Time distributions
        int time_bins       = 100;
        hist.step_prim_time = new TH1D("",
                                       "Primary step time",
                                       time_bins,
                                       0,
                                       data_limits.max_time * plot_margin);
        hist.step_sec_time  = new TH1D("",
                                      "Secondary step time",
                                      time_bins,
                                      0,
                                      data_limits.max_time * plot_margin);
        hist.vtx_prim_time  = new TH1D("",
                                      "Primary vertex time",
                                      time_bins,
                                      0,
                                      data_limits.max_time * plot_margin);
        hist.vtx_sec_time   = new TH1D("",
                                     "Secondary vertex time",
                                     time_bins,
                                     0,
                                     data_limits.max_time * plot_margin);

        // Length distributions
        hist.prim_length      = new TH1D("",
                                    "Primary length",
                                    time_bins,
                                    0,
                                    data_limits.max_trk_length * plot_margin);
        hist.sec_length       = new TH1D("",
                                   "Secondary length",
                                   time_bins,
                                   0,
                                   data_limits.max_trk_length * plot_margin);
        hist.prim_step_length = new TH1D("",
                                         "Prim. step length",
                                         time_bins,
                                         0,
                                         data_limits.max_length * plot_margin);
        hist.sec_step_length  = new TH1D("",
                                        "Sec. step length",
                                        time_bins,
                                        0,
                                        data_limits.max_length * plot_margin);

        // Detector scoring
        hist.sitracker_edep = new TH1D("",
                                       "Si tracker energy deposition",
                                       hist.sd_edep_bins,
                                       0,
                                       data_limits.max_sd_energy * plot_margin);

        hist.emcalo_edep = new TH1D("",
                                    "EM calorimeter energy deposition",
                                    hist.sd_edep_bins,
                                    0,
                                    data_limits.max_sd_energy * plot_margin);

        hist.sitracker_nsteps
            = new TH1D("",
                       "Si tracker steps per event",
                       hist.sd_steps_bins,
                       0,
                       data_limits.max_sd_num_steps * plot_margin);

        hist.emcalo_nsteps
            = new TH1D("",
                       "EM calorimeter steps per event",
                       hist.sd_steps_bins,
                       0,
                       data_limits.max_sd_num_steps * plot_margin);
    }
}

//---------------------------------------------------------------------------//
/*!
 * Set up global cumulative bin arrays.
 */
void initialize_cumulative_bins()
{
    for (auto& graph : vg::graphs)
    {
        graph.r_bins.resize(graph.r_n_bins);
        graph.z_bins.resize(graph.z_n_bins);

        for (int i = 0; i < graph.r_n_bins; i++)
        {
            const int ith_bin_pos = i * graph.r_bin_size;

            graph.r_bins[i].min    = ith_bin_pos;
            graph.r_bins[i].center = ith_bin_pos + graph.r_bin_half_width;
            graph.r_bins[i].max    = ith_bin_pos + graph.r_bin_size;
            graph.r_bins[i].value  = 0;
        }

        for (int i = 0; i < graph.z_n_bins; i++)
        {
            const int ith_bin_pos = i * graph.z_bin_size;

            graph.z_bins[i].min    = graph.z_bin_min + ith_bin_pos;
            graph.z_bins[i].center = graph.z_bins[i].min
                                     + graph.z_bin_half_width;
            graph.z_bins[i].max   = graph.z_bins[i].min + graph.z_bin_size;
            graph.z_bins[i].value = 0;
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Set up local cumulative bin arrays.
 */
void initialize_cumulative_bins(vg::GraphData& graph)
{
    graph.r_bins.resize(graph.r_n_bins);
    graph.z_bins.resize(graph.z_n_bins);

    for (int i = 0; i < graph.r_n_bins; i++)
    {
        const int ith_bin_pos = i * graph.r_bin_size;

        graph.r_bins[i].min    = ith_bin_pos;
        graph.r_bins[i].center = ith_bin_pos + graph.r_bin_half_width;
        graph.r_bins[i].max    = ith_bin_pos + graph.r_bin_size;
        graph.r_bins[i].value  = 0;
    }

    for (int i = 0; i < graph.z_n_bins; i++)
    {
        const int ith_bin_pos = i * graph.z_bin_size;

        graph.z_bins[i].min    = graph.z_bin_min + ith_bin_pos;
        graph.z_bins[i].center = graph.z_bins[i].min + graph.z_bin_half_width;
        graph.z_bins[i].max    = graph.z_bins[i].min + graph.z_bin_size;
        graph.z_bins[i].value  = 0;
    }
}
//---------------------------------------------------------------------------//
} // namespace vg
