//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file ValidationsHelper.hh
//! \brief Helper functions for the main validation plot macro
//---------------------------------------------------------------------------//
/*!
 * \note
 * \c loop(...) : Runs the main loop over the events tree, which is composed by
 * smaller loops and functions to fill all the necessary data in \c vg::hist
 * and \c vg::graph .
 */
//---------------------------------------------------------------------------//
#pragma once

#include "ValidationGlobals.hh"

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>
#include <TStyle.h>
#include <TCanvas.h>

//---------------------------------------------------------------------------//
// INNER LOOPS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Inner loop over primaries.
 */
void loop_primaries(rootdata::Event& event, vg::MC mc_enum)
{
    auto& hist  = vg::histograms[mc_enum];
    auto& graph = vg::graphs[mc_enum];

    for (const auto& primary : event.primaries)
    {
        vg::total_num_steps += primary.number_of_steps;
        vg::num_steps_per_event += primary.number_of_steps;

        // Step histograms
        hist.n_steps_prim->Fill(primary.number_of_steps);
        hist.prim_edep->Fill(primary.energy_dep);

        TParticlePDG* particle    = vg::pdg_db.GetParticle(primary.pdg);
        const char* particle_name = particle ? particle->GetName() : "unknown";
        vg::particle_map.insert({primary.pdg, particle_name});

        // Vertex time
        hist.vtx_prim_time->Fill(primary.vertex_global_time);

        // track length
        hist.prim_length->Fill(primary.length);

        for (const auto& step : primary.steps)
        {
            hist.prim_step_edep->Fill(step.energy_loss);

            std::string proc_name = rootdata::to_process_name(step.process_id);
            hist.steps_process->Fill(proc_name.c_str(), 1);
            hist.particle_process->Fill(proc_name.c_str(), particle_name, 1);
            vg::process_map.insert({step.process_id, proc_name});

            // Step time and length
            hist.step_prim_time->Fill(step.global_time);
            hist.prim_step_length->Fill(step.length);
        }

        ROOT::Math::XYZVector xyz(primary.vertex_position.x,
                                  primary.vertex_position.y,
                                  primary.vertex_position.z);
        const double          xyz_rho = xyz.rho();

        // >>> Cumulative plots
        for (int i = 0; i < graph.r_n_bins; i++)
        {
            if (graph.r_bins[i].min < xyz_rho && graph.r_bins[i].max > xyz_rho)
            {
                graph.r_bins[i].value += primary.energy_dep;
                break;
            }
        }

        for (int i = 0; i < graph.z_n_bins; i++)
        {
            if (graph.z_bins[i].min < xyz.z() && graph.z_bins[i].max > xyz.z())
            {
                graph.z_bins[i].value += primary.energy_dep;
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Inner loop over secondaries.
 */
void loop_secondaries(rootdata::Event& event, vg::MC mc_enum)
{
    auto& hist  = vg::histograms[mc_enum];
    auto& graph = vg::graphs[mc_enum];

    for (const auto& secondary : event.secondaries)
    {
        vg::total_num_steps += secondary.number_of_steps;
        vg::num_steps_per_event += secondary.number_of_steps;

        // Step histograms
        hist.n_steps_sec->Fill(secondary.number_of_steps);

        // Energy histograms
        hist.sec_edep->Fill(secondary.energy_dep);
        hist.sec_energy->Fill(secondary.vertex_energy);

        // Vertex histograms
        ROOT::Math::XYZVector xyz(secondary.vertex_position.x,
                                  secondary.vertex_position.y,
                                  secondary.vertex_position.z);
        const double          xyz_rho = xyz.rho();

        hist.sec_vtx_x->Fill(secondary.vertex_position.x);
        hist.sec_vtx_y->Fill(secondary.vertex_position.y);
        hist.sec_vtx_z->Fill(secondary.vertex_position.z);
        hist.sec_vtx_r->Fill(xyz_rho);
        hist.sec_vtx_theta->Fill(xyz.phi()); // Phi = polar theta

        // Direction histograms
        hist.sec_dir_x->Fill(secondary.vertex_direction.x);
        hist.sec_dir_y->Fill(secondary.vertex_direction.y);
        hist.sec_dir_z->Fill(secondary.vertex_direction.z);

        // Vertex time
        hist.vtx_sec_time->Fill(secondary.vertex_global_time);

        // track length
        hist.sec_length->Fill(secondary.length);

        // Particle and process map
        TParticlePDG* particle    = vg::pdg_db.GetParticle(secondary.pdg);
        const char* particle_name = particle ? particle->GetName() : "unknown";
        vg::particle_map.insert({secondary.pdg, particle_name});

        for (const auto& step : secondary.steps)
        {
            hist.sec_step_edep->Fill(step.energy_loss);

            std::string proc_name = rootdata::to_process_name(step.process_id);
            hist.steps_process->Fill(proc_name.c_str(), 1);
            hist.particle_process->Fill(proc_name.c_str(), particle_name, 1);
            vg::process_map.insert({step.process_id, proc_name});

            // Step time
            hist.step_sec_time->Fill(step.global_time);
            hist.sec_step_length->Fill(step.length);
        }

        // >>> Cumulative plots
        for (int i = 0; i < graph.r_n_bins; i++)
        {
            if (graph.r_bins[i].min < xyz_rho && graph.r_bins[i].max > xyz_rho)
            {
                /*
                 * In case we need to find out the cumulative energy
                 * deposition by killing secondaries on Celeritas, a quick
                 * workaround is to replace energy_dep by vertex_energy
                 */
                graph.r_bins[i].value += secondary.energy_dep;
                break;
            }
        }
        for (int i = 0; i < graph.z_n_bins; i++)
        {
            if (graph.z_bins[i].min < xyz.x() && graph.z_bins[i].max > xyz.x())
            {
                graph.z_bins[i].value += secondary.energy_dep;
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * Fill sensitive detector plots.
 */
void sensitive_detectors(rootdata::Event& event)
{
    auto&       hist      = vg::histograms[vg::MC::G4];
    const auto& detectors = event.sensitive_detectors;

    if (detectors.size() != 2)
    {
        std::cerr << std::endl;
        std::cerr << "Warning: sensitive_detectors(event) function call can't "
                     "handle this case: event.sensitive_detectors.size() = "
                  << detectors.size() << std::endl;
        return;
    }

    hist.sitracker_edep->Fill(detectors.at(0).energy_deposition);
    hist.sitracker_nsteps->Fill(detectors.at(0).number_of_steps);

    hist.emcalo_edep->Fill(detectors.at(1).energy_deposition);
    hist.emcalo_nsteps->Fill(detectors.at(1).number_of_steps);
}

//---------------------------------------------------------------------------//
// MAIN LOOP
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Main loop to fill all \c vg::histograms and \c vg::graphs objects.
 */
void loop(TFile* input_file, vg::MC mc_enum)
{
    auto& hist  = vg::histograms[mc_enum];
    auto& graph = vg::graphs[mc_enum];

    TTree*           event_tree = (TTree*)input_file->Get("events");
    rootdata::Event* event      = nullptr;
    event_tree->SetBranchAddress("event", &event);

    // Add to global number of events
    vg::total_num_events += event_tree->GetEntries();

    // Set up progress indicator
    const float percent_increment = 1; // Print msg at every increment [%]
    const int   events_per_print  = (percent_increment / 100)
                                 * event_tree->GetEntries();
    int events_per_print_counter = 0; // Addition is better than modulo
    int n_printed_msgs = 0; // One printed msg for every percent increment
    std::cout << "Processing " << input_file->GetName() << ": 0%";
    std::cout.flush();

    for (int i = 0; i < event_tree->GetEntries(); i++)
    {
        event_tree->GetEntry(i);

        if (++events_per_print_counter == events_per_print)
        {
            // Reached the number of events per print
            // Increment number of printed messages and reset counter
            n_printed_msgs++;
            events_per_print_counter = 0;
            std::cout << "\rProcessing " << input_file->GetName() << ": "
                      << n_printed_msgs * percent_increment << "%";
            std::cout.flush();
        }

        // Reset counter for each event
        vg::num_steps_per_event = 0;

        // Loop over event data and fill validation global (vg) data
        loop_primaries(*event, mc_enum);
        loop_secondaries(*event, mc_enum);

        // if (!event->sensitive_detectors.empty())
        // {
        // sensitive_detectors(*event);
        // }

        // Step histograms
        hist.n_secondaries->Fill(event->secondaries.size());
        hist.n_steps_evt->Fill(vg::num_steps_per_event);
    }

    std::cout << "\rProcessing " << input_file->GetName() << ": 100%"
              << std::endl;
    std::cout.flush();

    // Set all the vg::graphs.TGraph* data points
    for (int i = 0; i < graph.r_n_bins; i++)
    {
        graph.r_bins[i].value /= event_tree->GetEntries() - 1;
        graph.r_bins[i].value /= graph.r_bin_size;

        graph.cumulative_r->SetPoint(
            i, graph.r_bins[i].center, graph.r_bins[i].value);
    }

    for (int i = 0; i < graph.z_n_bins; i++)
    {
        graph.z_bins[i].value /= event_tree->GetEntries() - 1;
        graph.z_bins[i].value /= graph.z_bin_size;

        graph.cumulative_z->SetPoint(
            i, graph.z_bins[i].center, graph.z_bins[i].value);
    }
}

//---------------------------------------------------------------------------//
// CANVASES
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Draw canvas with step plots.
 */
void draw_canvas_step()
{
    auto& hist_g4  = vg::histograms[vg::MC::G4];
    auto& hist_cel = vg::histograms[vg::MC::Cel];

    const auto celeritas_color = kAzure + 1;
    hist_cel.n_steps_prim->SetLineColor(celeritas_color);
    hist_cel.n_secondaries->SetLineColor(celeritas_color);
    hist_cel.n_steps_sec->SetLineColor(celeritas_color);
    hist_cel.n_steps_evt->SetLineColor(celeritas_color);

    TCanvas* canvas_steps = new TCanvas("steps", "steps", 1600, 650);
    canvas_steps->Divide(4, 2); // 3 columns, 2 rows

    // Row 1
    canvas_steps->cd(1);
    gPad->SetLogy();
    hist_g4.n_steps_prim->GetXaxis()->SetTitle("Number of steps");
    hist_g4.n_steps_prim->GetXaxis()->CenterTitle();
    hist_g4.n_steps_prim->Draw();
    hist_cel.n_steps_prim->Draw("sames");

    canvas_steps->cd(2);
    hist_g4.n_secondaries->GetXaxis()->SetTitle("Number of secondaries");
    hist_g4.n_secondaries->GetXaxis()->CenterTitle();
    hist_g4.n_secondaries->Draw();
    hist_cel.n_secondaries->Draw("sames");

    canvas_steps->cd(3);
    gPad->SetLogy();
    hist_g4.n_steps_sec->GetXaxis()->SetTitle("Number of steps");
    hist_g4.n_steps_sec->GetXaxis()->CenterTitle();
    hist_g4.n_steps_sec->Draw();
    hist_cel.n_steps_sec->Draw("sames");

    canvas_steps->cd(4);
    gPad->SetLogy();
    hist_g4.n_steps_evt->GetXaxis()->SetTitle("Number of steps");
    hist_g4.n_steps_evt->GetXaxis()->CenterTitle();
    hist_g4.n_steps_evt->Draw();
    hist_cel.n_steps_evt->Draw("sames");

    // Row 2
    canvas_steps->cd(5);
    gPad->SetLogy();
    gPad->SetBottomMargin(vg::bottom_margin);
    hist_g4.steps_process->Draw();
    hist_g4.steps_process->GetXaxis()->SetRange(1, vg::process_map.size());
    hist_g4.steps_process->SetLabelOffset(vg::label_offset);
    hist_g4.steps_process->SetLabelSize(vg::label_size);

    canvas_steps->cd(6);
    gStyle->SetPalette(kViridis);
    gPad->SetLeftMargin(vg::left_margin);
    gPad->SetRightMargin(vg::right_margin);
    gPad->SetBottomMargin(vg::bottom_margin);
    gStyle->SetPaintTextFormat("1.2g"); // Set scientific notation inside bins
    hist_g4.particle_process->Scale(1. / hist_g4.particle_process->Integral());
    hist_g4.particle_process->Draw("ncolz text");
    hist_g4.particle_process->GetYaxis()->SetRangeUser(
        0, vg::particle_map.size());
    hist_g4.particle_process->GetXaxis()->SetRange(1, vg::process_map.size());
    hist_g4.particle_process->SetLabelOffset(vg::label_offset);
    hist_g4.particle_process->SetLabelSize(vg::label_size);
    hist_g4.particle_process->GetYaxis()->SetLabelSize(vg::label_size);
    hist_g4.particle_process->SetMarkerColor(vg::marker_color);
    hist_g4.particle_process->SetMarkerSize(vg::marker_size);

    canvas_steps->cd(7);
    gPad->SetLogy();
    gPad->SetBottomMargin(vg::bottom_margin);
    hist_cel.steps_process->Draw();
    hist_cel.steps_process->GetXaxis()->SetRange(1, vg::process_map.size());
    hist_cel.steps_process->SetLabelOffset(vg::label_offset);
    hist_cel.steps_process->SetLabelSize(vg::label_size);

    canvas_steps->cd(8);
    gStyle->SetPalette(kViridis);
    gPad->SetLeftMargin(vg::left_margin);
    gPad->SetRightMargin(vg::right_margin);
    gPad->SetBottomMargin(vg::bottom_margin);
    gStyle->SetPaintTextFormat("1.2g"); // Set scientific notation inside bins
    hist_cel.particle_process->Scale(1.
                                     / hist_cel.particle_process->Integral());
    hist_cel.particle_process->Draw("ncolz text");
    hist_cel.particle_process->GetYaxis()->SetRangeUser(
        0, vg::particle_map.size());
    hist_cel.particle_process->GetXaxis()->SetRange(1, vg::process_map.size());
    hist_cel.particle_process->SetLabelOffset(vg::label_offset);
    hist_cel.particle_process->SetLabelSize(vg::label_size);
    hist_cel.particle_process->GetYaxis()->SetLabelSize(vg::label_size);
    hist_cel.particle_process->SetMarkerColor(vg::marker_color);
    hist_cel.particle_process->SetMarkerSize(vg::marker_size);

    canvas_steps->SaveAs("canvas_steps.pdf");
}

//---------------------------------------------------------------------------//
/*!
 * Draw canvas with energy plots.
 */
void draw_canvas_energy()
{
    auto& hist_g4  = vg::histograms[vg::MC::G4];
    auto& hist_cel = vg::histograms[vg::MC::Cel];

    const auto celeritas_color = kAzure + 1;
    hist_cel.prim_edep->SetLineColor(celeritas_color);
    hist_cel.sec_edep->SetLineColor(celeritas_color);
    hist_cel.sec_energy->SetLineColor(celeritas_color);
    hist_cel.prim_step_edep->SetLineColor(celeritas_color);
    hist_cel.sec_step_edep->SetLineColor(celeritas_color);

    TCanvas* canvas_energy = new TCanvas("energy", "energy", 1050, 600);
    canvas_energy->Divide(3, 2); // 3 columns, 2 rows

    canvas_energy->cd(1);
    gPad->SetLogy();
    hist_g4.prim_edep->GetXaxis()->SetTitle("Energy deposition [MeV]");
    hist_g4.prim_edep->GetXaxis()->CenterTitle();
    hist_g4.prim_edep->Draw();
    hist_cel.prim_edep->Draw("sames");

    canvas_energy->cd(2);
    gPad->SetLogy();
    hist_g4.sec_edep->GetXaxis()->SetTitle("Energy deposition [MeV]");
    hist_g4.sec_edep->GetXaxis()->CenterTitle();
    hist_g4.sec_edep->Draw();
    hist_cel.sec_edep->Draw("sames");

    canvas_energy->cd(3);
    gPad->SetLogy();
    hist_g4.sec_energy->GetXaxis()->SetTitle("Energy [MeV]");
    hist_g4.sec_energy->GetXaxis()->CenterTitle();
    hist_g4.sec_energy->Draw();
    hist_cel.sec_energy->Draw("sames");

    // Row 2
    canvas_energy->cd(4);
    hist_g4.prim_step_edep->GetXaxis()->SetTitle("Energy deposition [MeV]");
    hist_g4.prim_step_edep->GetXaxis()->CenterTitle();
    hist_g4.prim_step_edep->Draw();
    hist_cel.prim_step_edep->Draw("sames");

    canvas_energy->cd(5);
    hist_g4.sec_step_edep->GetXaxis()->SetTitle("Energy deposition [MeV]");
    hist_g4.sec_step_edep->GetXaxis()->CenterTitle();
    hist_g4.sec_step_edep->Draw();
    hist_cel.sec_step_edep->Draw("sames");

    canvas_energy->SaveAs("canvas_energy.pdf");
}

//---------------------------------------------------------------------------//
/*!
 * Draw canvas with vertex plots.
 */
void draw_canvas_vertex()
{
    auto& hist_g4  = vg::histograms[vg::MC::G4];
    auto& hist_cel = vg::histograms[vg::MC::Cel];

    const auto celeritas_color = kAzure + 1;
    hist_cel.sec_vtx_x->SetLineColor(celeritas_color);
    hist_cel.sec_vtx_y->SetLineColor(celeritas_color);
    hist_cel.sec_vtx_z->SetLineColor(celeritas_color);
    hist_cel.sec_dir_x->SetLineColor(celeritas_color);
    hist_cel.sec_dir_y->SetLineColor(celeritas_color);
    hist_cel.sec_dir_z->SetLineColor(celeritas_color);
    hist_cel.sec_vtx_r->SetLineColor(celeritas_color);
    hist_cel.sec_vtx_theta->SetLineColor(celeritas_color);

    TCanvas* canvas_vertex = new TCanvas("vertex", "vertex", 1050, 600);
    canvas_vertex->Divide(3, 2); // 3 columns, 2 rows

    // Row 1
    canvas_vertex->cd(1);
    hist_g4.sec_vtx_x->GetXaxis()->SetTitle("Vertex x [cm]");
    hist_g4.sec_vtx_x->GetXaxis()->CenterTitle();
    hist_g4.sec_vtx_x->Draw();
    hist_cel.sec_vtx_x->Draw("sames");

    canvas_vertex->cd(2);
    hist_g4.sec_vtx_y->GetXaxis()->SetTitle("Vertex y [cm]");
    hist_g4.sec_vtx_y->GetXaxis()->CenterTitle();
    hist_g4.sec_vtx_y->Draw();
    hist_cel.sec_vtx_y->Draw("sames");

    canvas_vertex->cd(3);
    hist_g4.sec_vtx_z->GetXaxis()->SetTitle("Vertex z [cm]");
    hist_g4.sec_vtx_z->GetXaxis()->CenterTitle();
    hist_g4.sec_vtx_z->Draw();
    hist_cel.sec_vtx_z->Draw("sames");

    // Row 2
    canvas_vertex->cd(4);
    hist_g4.sec_dir_x->GetXaxis()->SetTitle("Vertex direction x");
    hist_g4.sec_dir_x->GetXaxis()->CenterTitle();
    hist_g4.sec_dir_x->Draw();
    hist_cel.sec_dir_x->Draw("sames");

    canvas_vertex->cd(5);
    hist_g4.sec_dir_y->GetXaxis()->SetTitle("Vertex direction y");
    hist_g4.sec_dir_y->GetXaxis()->CenterTitle();
    hist_g4.sec_dir_y->Draw();
    hist_cel.sec_dir_y->Draw("sames");

    canvas_vertex->cd(6);
    hist_g4.sec_dir_z->GetXaxis()->SetTitle("Vertex direction z");
    hist_g4.sec_dir_z->GetXaxis()->CenterTitle();
    hist_g4.sec_dir_z->Draw();
    hist_cel.sec_dir_z->Draw("sames");

#if 0
    // Row 2 [R and theta]
    canvas_vertex->cd(4);
    hist_g4.sec_vtx_r->GetXaxis()->SetTitle("Vertex r [cm]");
    hist_g4.sec_vtx_r->GetXaxis()->CenterTitle();
    hist_g4.sec_vtx_r->Draw();
    hist_cel.sec_vtx_r->Draw("sames");

    canvas_vertex->cd(5);
    hist_g4.sec_vtx_theta->GetXaxis()->SetTitle("Vertex #theta [rad]");
    hist_g4.sec_vtx_theta->GetXaxis()->CenterTitle();
    hist_g4.sec_vtx_theta->Draw();
    hist_cel.sec_vtx_theta->Draw("sames");
#endif

    canvas_vertex->SaveAs("canvas_vertex.pdf");
}

//---------------------------------------------------------------------------//
/*!
 * Draw canvas with sensitive detector plots.
 */
void draw_canvas_sensitive_detectors()
{
    // TODO: Add Celeritas plot comparison
    auto hist = vg::histograms[vg::MC::G4];

    TCanvas* canvas_sd
        = new TCanvas("sensitive detectors", "sensitive detectors", 700, 600);
    canvas_sd->Divide(2, 2); // 2 columns, 2 rows

    // Row 1
    canvas_sd->cd(1);
    gPad->SetLogy();
    hist.sitracker_edep->Draw();
    hist.sitracker_edep->GetXaxis()->SetTitle("Energy deposition [MeV]");
    hist.sitracker_edep->GetXaxis()->CenterTitle();

    canvas_sd->cd(2);
    gPad->SetLogy();
    hist.emcalo_edep->Draw();
    hist.emcalo_edep->GetXaxis()->SetTitle("Energy deposition [MeV]");
    hist.emcalo_edep->GetXaxis()->CenterTitle();

    // Row 2
    canvas_sd->cd(3);
    gPad->SetLogy();
    hist.sitracker_nsteps->Draw();
    hist.sitracker_nsteps->GetXaxis()->SetTitle("Number of steps");
    hist.sitracker_nsteps->GetXaxis()->CenterTitle();

    canvas_sd->cd(4);
    gPad->SetLogy();
    hist.emcalo_nsteps->Draw();
    hist.emcalo_nsteps->GetXaxis()->SetTitle("Number of steps");
    hist.emcalo_nsteps->GetXaxis()->CenterTitle();
}

//---------------------------------------------------------------------------//
/*!
 * Draw canvas with cumulative distributions.
 */
void draw_canvas_cumulative()
{
    auto& graph_g4  = vg::graphs[vg::MC::G4];
    auto& graph_cel = vg::graphs[vg::MC::Cel];

    const auto celeritas_color = kAzure + 1;
    graph_cel.cumulative_r->SetLineColor(celeritas_color);
    graph_cel.cumulative_z->SetLineColor(celeritas_color);

    TCanvas* canvas_cumulative
        = new TCanvas("cumulative", "cumulative", 1050, 500);
    canvas_cumulative->Divide(2, 1); // 2 columns, 1 row

    canvas_cumulative->cd(1);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLeftMargin(vg::left_margin);
    graph_g4.cumulative_r->SetLineWidth(2);
    graph_g4.cumulative_r->SetTitle("Cumulative radial energy deposition");
    graph_g4.cumulative_r->GetXaxis()->SetTitle("r (cylindrical) [cm]");
    graph_g4.cumulative_r->GetXaxis()->CenterTitle();
    graph_g4.cumulative_r->GetYaxis()->SetTitle("Energy deposition [MeV]");
    graph_g4.cumulative_r->GetYaxis()->SetTitleOffset(1.1);
    graph_g4.cumulative_r->GetYaxis()->CenterTitle();
    graph_g4.cumulative_r->Draw("AL");
    graph_cel.cumulative_r->Draw("L sames");
    gPad->RedrawAxis();

    canvas_cumulative->cd(2);
    gPad->SetGridx();
    gPad->SetGridy();
    gPad->SetLeftMargin(vg::left_margin);
    graph_g4.cumulative_z->SetLineWidth(2);
    graph_g4.cumulative_z->SetTitle("Cumulative z energy deposition");
    graph_g4.cumulative_z->GetXaxis()->SetTitle("z [cm]");
    graph_g4.cumulative_z->GetXaxis()->CenterTitle();
    graph_g4.cumulative_z->GetYaxis()->SetTitle("Energy deposition [MeV]");
    graph_g4.cumulative_z->GetYaxis()->SetTitleOffset(1.1);
    graph_g4.cumulative_z->GetYaxis()->CenterTitle();
    graph_g4.cumulative_z->Draw("AL");
    graph_cel.cumulative_z->Draw("L sames");
    gPad->RedrawAxis();
}

//---------------------------------------------------------------------------//
/*!
 * Draw canvas with time distributions.
 */
void draw_canvas_time()
{
    auto& hist_g4  = vg::histograms[vg::MC::G4];
    auto& hist_cel = vg::histograms[vg::MC::Cel];

    const auto celeritas_color = kAzure + 1;
    hist_cel.vtx_prim_time->SetLineColor(celeritas_color);
    hist_cel.vtx_sec_time->SetLineColor(celeritas_color);
    hist_cel.step_prim_time->SetLineColor(celeritas_color);
    hist_cel.step_sec_time->SetLineColor(celeritas_color);

    TCanvas* canvas_time = new TCanvas("Global time", "Global time", 700, 600);
    canvas_time->Divide(2, 2); // 2 columns, 2 rows

    // Row 1
    canvas_time->cd(1);
    hist_g4.vtx_prim_time->SetTitle("Primary vertex global time");
    hist_g4.vtx_prim_time->GetXaxis()->SetTitle("time [s]");
    hist_g4.vtx_prim_time->GetXaxis()->CenterTitle();
    hist_g4.vtx_prim_time->Draw();
    hist_cel.vtx_prim_time->Draw("sames");

    canvas_time->cd(2);
    hist_g4.vtx_sec_time->SetTitle("Secondary vertex global time");
    hist_g4.vtx_sec_time->GetXaxis()->SetTitle("time [s]");
    hist_g4.vtx_sec_time->GetXaxis()->CenterTitle();
    hist_g4.vtx_sec_time->Draw();
    hist_cel.vtx_sec_time->Draw("sames");

    // Row 2
    canvas_time->cd(3);
    gPad->SetLogy(1);
    hist_g4.step_prim_time->SetTitle("Primary step global time");
    hist_g4.step_prim_time->GetXaxis()->SetTitle("time [s]");
    hist_g4.step_prim_time->GetXaxis()->CenterTitle();
    hist_g4.step_prim_time->Draw();
    hist_cel.step_prim_time->Draw("sames");

    canvas_time->cd(4);
    gPad->SetLogy(1);
    hist_g4.step_sec_time->SetTitle("Secondary step global time");
    hist_g4.step_sec_time->GetXaxis()->SetTitle("time [s]");
    hist_g4.step_sec_time->GetXaxis()->CenterTitle();
    hist_g4.step_sec_time->Draw();
    hist_cel.step_sec_time->Draw("sames");

    canvas_time->SaveAs("canvas_time.pdf");
}
//---------------------------------------------------------------------------//
/*!
 * Draw canvas with length distributions.
 */
void draw_canvas_length()
{
    auto& hist_g4  = vg::histograms[vg::MC::G4];
    auto& hist_cel = vg::histograms[vg::MC::Cel];

    const auto celeritas_color = kAzure + 1;
    hist_cel.prim_length->SetLineColor(celeritas_color);
    hist_cel.sec_length->SetLineColor(celeritas_color);
    hist_cel.prim_step_length->SetLineColor(celeritas_color);
    hist_cel.sec_step_length->SetLineColor(celeritas_color);

    TCanvas* canvas_time
        = new TCanvas("Track length", "Track length", 700, 600);
    canvas_time->Divide(2, 2); // 2 columns, 2 rows

    // Row 1
    canvas_time->cd(1);
    gPad->SetLogy(1);
    hist_g4.prim_length->SetTitle("Primary track length");
    hist_g4.prim_length->GetXaxis()->SetTitle("length [cm]");
    hist_g4.prim_length->GetXaxis()->CenterTitle();
    hist_g4.prim_length->Draw();
    hist_cel.prim_length->Draw("sames");

    canvas_time->cd(2);
    gPad->SetLogy(1);
    hist_g4.sec_length->SetTitle("Secondary track length");
    hist_g4.sec_length->GetXaxis()->SetTitle("length [cm]");
    hist_g4.sec_length->GetXaxis()->CenterTitle();
    hist_g4.sec_length->Draw();
    hist_cel.sec_length->Draw("sames");

    // Row 2
    canvas_time->cd(3);
    gPad->SetLogy(1);
    hist_g4.prim_step_length->SetTitle("Primary step length");
    hist_g4.prim_step_length->GetXaxis()->SetTitle("length [cm]");
    hist_g4.prim_step_length->GetXaxis()->CenterTitle();
    hist_g4.prim_step_length->Draw();
    hist_cel.prim_step_length->Draw("sames");

    canvas_time->cd(4);
    gPad->SetLogy(1);
    hist_g4.sec_step_length->SetTitle("Secondary step length");
    hist_g4.sec_step_length->GetXaxis()->SetTitle("length [cm]");
    hist_g4.sec_step_length->GetXaxis()->CenterTitle();
    hist_g4.sec_step_length->Draw();
    hist_cel.sec_step_length->Draw("sames");

    canvas_time->SaveAs("canvas_length.pdf");
}
