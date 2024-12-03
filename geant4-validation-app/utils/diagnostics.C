//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file diagnostics.C
//! \brief Create diagnostics plots.
//---------------------------------------------------------------------------//
/*!
 * This is a generic tool to loop over data and print C++ arrays to the
 * terminal. This allows looping over the full data once and pasting the final
 * results as arrays in simple plotting macros.
 */
//---------------------------------------------------------------------------//
#include <iomanip>
#include <TBranch.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>

#include "../src/RootData.hh"
#include "ValidationGlobals.hh"

//---------------------------------------------------------------------------//
/*!
 * Enums for safely selecting particles and MC codes.
 */
enum MC
{
    G4,
    Cel,
    mc_size
};

enum PID
{
    e_plus,
    e_minus,
    photon,
    pid_size
};

enum PDG
{
    pdg_e_plus = -11,
    pdg_e_minus = 11,
    pdg_gamma = 22
};

//---------------------------------------------------------------------------//
/*!
 * Fwd declaration of currently implemented diagnostics.
 */
void print_steps_per_track(TFile* input_file);
void print_edep(TFile* input_file);
void export_particle_process(TFile* input_file);

//---------------------------------------------------------------------------//
// MAIN MACRO
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Print diagnostics data.
 *
 * The printed arrays can then be used alongside a Celeritas output to
 * create comparison plots.
 *
 * Usage:
 * root[0] .x diagnostics.C("/path/to/g4_output.root")
 */
void diagnostics(char const* input)
{
    // Load rootdata shared library
    gSystem->Load("../build/librootdata");

    // print_edep(new TFile(input, "read"));
    //  export_particle_process(new TFile(input, "rea.qd"));
    print_steps_per_track(new TFile(input, "read"));
}

//---------------------------------------------------------------------------//
// Helper functions to print processed data as C++ arrays
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Print histogram bins as a C++ array.
 */
void print_data_array(TH1D* histogram, std::string declaration)
{
    int const max_nbins = histogram->GetNbinsX();
    std::cout << declaration << "[" << max_nbins << "] = {" << std::endl;

    for (int i = 0; i < max_nbins; i++)
    {
        std::cout << std::setprecision(15) << histogram->GetBinContent(i);

        if (i < max_nbins - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "};\n" << std::endl;
}

//---------------------------------------------------------------------------//
/*!
 * Print C++ array.
 */
void print_data_array(double array[], std::size_t size, std::string declaration)
{
    std::cout << declaration << "[" << size << "] = { ";

    for (int i = 0; i < size; i++)
    {
        std::cout << array[i];
        if (i < size - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "};" << std::endl;
}

//---------------------------------------------------------------------------//
/*!
 * Print histogram bins as a C++ arrays.
 */
void print_data_array(std::vector<vg::GraphData::Bin>& bins,
                      std::string declaration)
{
    // >>>  Print R values
    double const n_bins = bins.size();
    std::cout << declaration << "_x[" << n_bins << "] = {" << std::endl;

    for (int i = 0; i < n_bins; i++)
    {
        std::cout << bins.at(i).center;

        if (i < n_bins - 1)
        {
            std::cout << ", ";
        }
    }

    std::cout << "};\n" << std::endl;
    std::cout << std::endl;
    std::cout << declaration << "_y[" << n_bins << "] = {" << std::endl;

    for (int i = 0; i < n_bins; i++)
    {
        std::cout << bins.at(i).value;

        if (i < n_bins - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "};\n" << std::endl;
}

//---------------------------------------------------------------------------//
// Individual diagnostics that are called by the main diagnostics() macro
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Print steps per track arrays for each particle type for comparing Geant4 and
 * Celeritas.
 */
void print_steps_per_track(TFile* input_file)
{
    TTree* event_tree = (TTree*)input_file->Get("events");
    rootdata::Event* event = nullptr;
    event_tree->SetBranchAddress("event", &event);

    TH1D* h_steps[mc_size][pid_size];

    for (int i = 0; i < mc_size; i++)
    {
        for (int j = 0; j < pid_size; j++)
        {
            // Initialize pointers
            // Start histogram binning at 1. This will make ROOT use bin[0] as
            // the underflow, which will store the number of tracks with 0
            // steps, leaving bin[1] = number of tracks with 1 step
            h_steps[i][j] = new TH1D("", "", 200, 1, 200);
        }
    }

    // Counters
    std::size_t total_steps[pid_size] = {0, 0, 0};
    std::size_t total_tracks[pid_size] = {0, 0, 0};
    std::string name[pid_size] = {"e+", "e-", "gamma"};

    // >>> Fill Geant4 histograms
    auto& h_steps_g4 = h_steps[MC::G4];

    for (int i = 0; i < event_tree->GetEntries(); i++)
    {
        // Load i-th event
        event_tree->GetEntry(i);

        std::cout << "\rProcessing event " << i;
        std::cout.flush();

        for (auto const& primary : event->primaries)
        {
            switch (primary.pdg)
            {
                case PDG::pdg_e_plus:
                    h_steps_g4[PID::e_plus]->Fill(primary.number_of_steps);
                    total_steps[PID::e_plus] += primary.number_of_steps;
                    total_tracks[PID::e_plus]++;
                    break;

                case PDG::pdg_e_minus:
                    h_steps_g4[PID::e_minus]->Fill(primary.number_of_steps);
                    total_steps[PID::e_minus] += primary.number_of_steps;
                    total_tracks[PID::e_minus]++;
                    break;

                case PDG::pdg_gamma:
                    h_steps_g4[PID::photon]->Fill(primary.number_of_steps);
                    total_steps[PID::photon] += primary.number_of_steps;
                    total_tracks[PID::photon]++;
                    break;
            }
        }

        for (auto const& secondary : event->secondaries)
        {
            switch (secondary.pdg)
            {
                case PDG::pdg_e_plus:
                    h_steps_g4[PID::e_plus]->Fill(secondary.number_of_steps);
                    total_steps[PID::e_plus] += secondary.number_of_steps;
                    total_tracks[PID::e_plus]++;
                    break;

                case PDG::pdg_e_minus:
                    h_steps_g4[PID::e_minus]->Fill(secondary.number_of_steps);
                    total_steps[PID::e_minus] += secondary.number_of_steps;
                    total_tracks[PID::e_minus]++;
                    break;

                case PDG::pdg_gamma:
                    h_steps_g4[PID::photon]->Fill(secondary.number_of_steps);
                    total_steps[PID::photon] += secondary.number_of_steps;
                    total_tracks[PID::photon]++;
                    break;
            }
        }
    }
    std::cout << "\rProcessing event [done]\n" << std::endl;

    std::cout << "Particle: Number of steps" << std::endl;
    for (int i = 0; i < pid_size; i++)
    {
        std::cout << name[i] << ": " << total_steps[i] << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Particle: Number of tracks" << std::endl;
    for (int i = 0; i < pid_size; i++)
    {
        std::cout << name[i] << ": " << total_tracks[i] << std::endl;
    }
    std::cout << std::endl;

    // >>> Print data arrays
    print_data_array(h_steps_g4[PID::e_plus], "const int cel_positron_steps");
    print_data_array(h_steps_g4[PID::e_minus], "const int cel_electron_steps");
    print_data_array(h_steps_g4[PID::photon], "const int cel_gamma_steps");
}

//---------------------------------------------------------------------------//
/*!
 * Print cumulative energy deposition. Current binning is for simple CMS.
 */
void print_edep(TFile* input_file)
{
    TTree* event_tree = (TTree*)input_file->Get("events");
    rootdata::Event* event = nullptr;
    event_tree->SetBranchAddress("event", &event);

    std::size_t const h_nbins = 1025;
    TH1D* h_edep = new TH1D("", "", h_nbins, -700, 700);

    std::vector<vg::GraphData::Bin> bins;
    for (int i = 0; i < h_edep->GetNbinsX(); i++)
    {
        vg::GraphData::Bin bin;
        bin.min = h_edep->GetBinLowEdge(i);
        bin.center = h_edep->GetBinCenter(i);
        bin.max = bin.min + h_edep->GetBinWidth(i);
        bin.value = 0;
        bins.push_back(std::move(bin));
    }

    double total_energy = 0;
    std::size_t const num_events = event_tree->GetEntries();

    // Create cumulative data array of all events for each z-bin
    std::vector<std::array<double, h_nbins>> vec_cumulative_data{};

    // >>> Fill histograms
    for (int i = 0; i < num_events; i++)
    {
        if (i % 100 == 0)
        {
            // Print status
            std::cout << "\rProcessing event " << i << std::flush;
        }

        // Load i-th event
        event_tree->GetEntry(i);

        std::array<double, h_nbins> cumulative_data{};

        // >>> Loop over primaries
        for (auto const& primary : event->primaries)
        {
            for (auto const& step : primary.steps)
            {
                total_energy += step.energy_loss;

                ROOT::Math::XYZVector xyz(
                    step.position.x, step.position.y, step.position.z);

                for (int j = 0; j < bins.size(); j++)
                {
                    auto const& bin = bins.at(j);

                    if (bin.min <= xyz.z() && bin.max > xyz.z())
                    {
                        double val = h_edep->GetBinContent(j);
                        double result = val + step.energy_loss;
                        h_edep->SetBinContent(j, result);
                        cumulative_data[j] = val + step.energy_loss;
                        break;
                    }
                }
            }
        }

        // >>> Loop over secondaries
        for (auto const& secondary : event->secondaries)
        {
            for (auto const& step : secondary.steps)
            {
                total_energy += step.energy_loss;

                ROOT::Math::XYZVector xyz(
                    step.position.x, step.position.y, step.position.z);

                for (int j = 0; j < bins.size(); j++)
                {
                    auto const& bin = bins.at(j);

                    if (bin.min <= xyz.z() && bin.max > xyz.z())
                    {
                        double val = h_edep->GetBinContent(j);
                        double result = val + step.energy_loss;
                        h_edep->SetBinContent(j, result);
                        cumulative_data[j] = val + step.energy_loss;
                        break;
                    }
                }
            }
        }

        // Add per-bin cumulative data to vector
        vec_cumulative_data.push_back(std::move(cumulative_data));
    }
    std::cout << "\rProcessing event [DONE]     " << std::endl;
    std::cout << std::flush;

    /*!
     * Calculate statistical error for each z-bin
     *
     * For any given bin, the energy deposition value is presented as
     * edep = x_avg +- Delta x_avg
     * Where:
     * Delta x_avg = sigma / sqrt(N)
     * sigma = sqrt(variance)
     * variance = (1/N-1) * Sum (x_i - x_avg)^2
     */
    std::cout << "Calculating variance..." << std::endl;

    double z_bin_error[h_nbins] = {};

    for (int bin_i = 0; bin_i < h_nbins; bin_i++)
    {
        // Calculate bin average
        double x_avg = 0;
        std::size_t num_entries = 0;

        for (auto const& cumulative_data : vec_cumulative_data)
        {
            if (cumulative_data[bin_i] > 0)
            {
                x_avg += cumulative_data[bin_i];
                num_entries++;
            }
        }

        if (num_entries)
        {
            // At least one non-zero entry, calculate average
            x_avg /= num_entries;
        }

        else
        {
            // No energy deposition in this bin
            // Set error to zero and move to next bin
            z_bin_error[bin_i] = 0;
            continue;
        }

        // Calculate variance per bin
        double variance = 0;
        for (auto const& cumulative_data : vec_cumulative_data)
        {
            if (cumulative_data[bin_i] > 0)
            {
                variance += std::pow((cumulative_data[bin_i] - x_avg), 2);
            }
        }

        if (num_entries > 1)
        {
            // More than 1 entry, calculate variance normally, avoiding a
            // division by zero (num_entries - 1 = 0)
            variance *= 1. / (num_entries - 1);
        }

        double sigma = std::sqrt(variance);
        z_bin_error[bin_i] = sigma / std::sqrt(num_entries);
    }
    std::cout << "Total energy = " << total_energy << std::endl;
    std::cout << std::endl;

    // >>> Print data arrays
    print_data_array(h_edep, "const double g4_edep_z");
    std::cout << std::endl;
    print_data_array(z_bin_error, h_nbins, "const double g4_edep_z_err");
}

//---------------------------------------------------------------------------//
/*!
 * Export particle vs. process histogram.
 */
void export_particle_process(TFile* input_file)
{
    TTree* event_tree = (TTree*)input_file->Get("events");
    rootdata::Event* event = nullptr;
    event_tree->SetBranchAddress("event", &event);

    auto h_particle_process = new TH2D("", "", 1, 0, 1, 1, 0, 1);

    for (int i = 0; i < event_tree->GetEntries(); i++)
    {
        if (i % 100 == 0)
        {
            std::cout << "\rProcessing event " << i << std::flush;
        }

        // Load i-th event
        event_tree->GetEntry(i);

        for (auto const& primary : event->primaries)
        {
            TParticlePDG* particle = vg::pdg_db.GetParticle(primary.pdg);
            vg::particle_map.insert({primary.pdg, particle->GetName()});

            for (auto const& step : primary.steps)
            {
                if (step.process_id == rootdata::ProcessId::transportation)
                {
                    // Skip transportation
                    continue;
                }

                std::string pname = rootdata::to_process_name(step.process_id);
                h_particle_process->Fill(pname.c_str(), particle->GetName(), 1);
                vg::process_map.insert({step.process_id, pname});
            }
        }

        for (auto const& secondary : event->secondaries)
        {
            TParticlePDG* particle = vg::pdg_db.GetParticle(secondary.pdg);
            vg::particle_map.insert({secondary.pdg, particle->GetName()});

            for (auto const& step : secondary.steps)
            {
                if (step.process_id == rootdata::ProcessId::transportation)
                {
                    // Skip transportation
                    continue;
                }

                std::string pname = rootdata::to_process_name(step.process_id);
                h_particle_process->Fill(pname.c_str(), particle->GetName(), 1);
                vg::process_map.insert({step.process_id, pname});
            }
        }
    }
    std::cout << "\rProcessing event [done]    " << std::endl;

    auto canvas = new TCanvas();
    gPad->SetLeftMargin(vg::left_margin);
    gPad->SetRightMargin(vg::right_margin);
    gPad->SetBottomMargin(vg::bottom_margin);
    h_particle_process->Draw("ncolz text");
    h_particle_process->GetYaxis()->SetRangeUser(0, vg::particle_map.size());
    h_particle_process->GetXaxis()->SetRange(1, vg::process_map.size());
    h_particle_process->SetLabelOffset(vg::label_offset);
    h_particle_process->SetLabelSize(vg::label_size);
    h_particle_process->GetYaxis()->SetLabelSize(vg::label_size);
    h_particle_process->SetMarkerColor(vg::marker_color);
    h_particle_process->SetMarkerSize(vg::marker_size);

    canvas->SaveAs("temp_g4_particle_process.C");
}
