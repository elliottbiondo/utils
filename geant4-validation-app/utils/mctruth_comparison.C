//----------------------------------*-C++-*----------------------------------//
// Copyright 2023-2025 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file mctruth_comparison.C
//! \brief Compare MC truth data between Geant4 and Celeritas
//---------------------------------------------------------------------------//
/*!
 * Usage:
 * $ root
 * root[0] .x mctruth_comparison.C("g4-out.root", "celeritas-out.root")
 *
 * `celer-sim` step MC truth output must be post-processed to be converted to
 * the same data structure of the Geant4 validation app with
 * \c `post_process_celeritas.C`.
 *
 * Update histogram info and select data accordingly in \c loop_tracks(...)
 * in the Helper functions and static variables section.
 *
 * Plot attributes are meant to be used with the Celeritas plot style. See
 * https://github.com/celeritas-project/benchmarks/blob/main/rootlogon.C
 */
//---------------------------------------------------------------------------//
#include <vector>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TSystem.h>
#include <TText.h>
#include <TTree.h>

#include "../../geant4-validation-app/src/RootData.hh"
#include "ProgressIndicator.hh"

//---------------------------------------------------------------------------//
//! Helper functions and static variables
//---------------------------------------------------------------------------//
// Loop over tracks
static bool const loop_primaries = true;
static bool const loop_secondaries = false;
// Histogram definition
static int const n_bins = 50;
static double const bin_min = 0;
static double const bin_max = 100e3;
static char const* hist_title = "Primary step length";
static char const* commit_hash = "[commit hash]";
static char const* x_axis_title = "Step length [cm]";
static char const* geant4_legend = "Geant4 v11.0.3";
static char const* celeritas_legend = "Celeritas v0.5*";
// Path to librootdata
static char const* librootdata = "../build/librootdata";

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// Loop over tracks (primaries and/or secondaries).
// Update function to fill histogram data accordingly.
void loop_tracks(std::vector<rootdata::Track> tracks, TH1D* hist)
{
    for (auto const& track : tracks)
    {
        for (auto const& step : track.steps)
            // Update data to be filled
            hist->Fill(step.length);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //
// Loop over events for a given ROOT file and populate histogram
void loop(std::string file, TH1D* hist)
{
    auto tfile = TFile::Open(file.c_str(), "read");
    auto event_tree = tfile->Get<TTree>("events");
    rootdata::Event* event = nullptr;
    event_tree->SetBranchAddress("event", &event);

    std::cout << "Open " << file << std::endl;
    ProgressIndicator progress(event_tree->GetEntries());

    for (auto i = 0; i < event_tree->GetEntries(); i++)
    {
        progress();
        event_tree->GetEntry(i);

        if (loop_primaries)
            loop_tracks(event->primaries, hist);
        if (loop_secondaries)
            loop_tracks(event->secondaries, hist);
    }

    tfile->Close();
}

//---------------------------------------------------------------------------//
/*!
 * Main function.
 */
void mctruth_comparison(std::string g4_rootfile, std::string cel_rootfile)
{
    // Load rootdata shared library
    gSystem->Load(librootdata);

    auto h_g4 = new TH1D("Geant4", "", n_bins, bin_min, bin_max);
    auto h_cel = new TH1D("Celeritas", "", n_bins, bin_min, bin_max);

    // Process data
    loop(g4_rootfile, h_g4);
    loop(cel_rootfile, h_cel);

    // Create relative error histograms
    auto h_g4_rel_err = new TH1D("G4 rel. err.", "", n_bins, bin_min, bin_max);
    auto h_g4_rel_err_3s
        = new TH1D("G4 rel. err. 3sigma", "", n_bins, bin_min, bin_max);
    for (int i = 0; i < n_bins; i++)
    {
        double error = h_g4->GetBinError(i);
        double value = h_g4->GetBinContent(i);
        double rel_err = value ? error / value : 0;

        h_g4_rel_err->SetBinContent(i, 0);
        h_g4_rel_err->SetBinError(i, rel_err * 100);
        h_g4_rel_err_3s->SetBinContent(i, 0);
        h_g4_rel_err_3s->SetBinError(i, 3 * rel_err * 100);
    }

    // Create relative difference histogram [(Geant4 - Celeritas) / Celeritas]
    auto h_rel_diff = (TH1D*)h_g4->Clone();
    h_rel_diff->Add(h_cel, -1);
    h_rel_diff->Divide(h_g4);
    h_rel_diff->Scale(100);  // In [%]

    // Create canvas
    auto canvas = new TCanvas("c1", "c1", 750, 600);
    canvas->Divide(1, 2);

    // Create top pad and move to it
    auto pad_top = new TPad("pad1", "", 0.0, 0.3, 1.0, 1.0);
    pad_top->SetBottomMargin(0.02);
    pad_top->SetLeftMargin(0.11);
    pad_top->Draw();
    pad_top->cd();

    // Histograms attributes
    auto const celeritas_color = kAzure + 1;
    h_cel->SetLineColor(celeritas_color);
    h_cel->SetLineWidth(2);
    h_g4->SetMarkerStyle(46);
    h_g4->SetMarkerSize(1.6);

    h_g4->GetXaxis()->SetLabelOffset(99);
    h_g4->GetYaxis()->SetLabelOffset(0.007);
    h_g4->GetYaxis()->CenterTitle();
    // h_g4->GetYaxis()->SetRangeUser(0, 10);

    // Draw histograms
    h_g4->Draw("PE2");
    h_cel->Draw("hist sames");

    auto legend_top = new TLegend(0.57, 0.46, 0.86, 0.86);
    legend_top->AddEntry(h_g4, geant4_legend, "p");
    legend_top->AddEntry(h_rel_diff, celeritas_legend, "l");
    legend_top->AddEntry(new TH1D(), "Statistical errors:", "f");
    legend_top->AddEntry(h_g4_rel_err, "1#sigma", "f");
    legend_top->AddEntry(h_g4_rel_err_3s, "3#sigma", "f");
    legend_top->SetMargin(0.27);
    legend_top->SetLineColor(kGray);
    legend_top->Draw();

    auto title_text = new TText(0.17, 0.92, hist_title);
    title_text->SetNDC();
    title_text->SetTextColor(kGray);
    title_text->Draw();

    auto commit_text = new TLatex(0.67, 0.92, commit_hash);
    commit_text->SetNDC();
    commit_text->SetTextColor(kGray);
    commit_text->Draw();

    // Redraw axis above the histogram lines
    pad_top->RedrawAxis();

    // Move back to canvas
    canvas->cd();

    // Create bottom pad and move to it
    auto pad_bottom = new TPad("pad2", "", 0.0, 0.0, 1.0, 0.3);
    pad_bottom->SetTopMargin(0.02);
    pad_bottom->SetBottomMargin(0.33);
    pad_bottom->SetLeftMargin(0.11);
    pad_bottom->Draw();
    pad_bottom->cd();

    h_g4_rel_err_3s->GetXaxis()->SetTitle(x_axis_title);
    h_g4_rel_err_3s->GetXaxis()->CenterTitle();
    h_g4_rel_err_3s->GetXaxis()->SetTitleSize(0.14);
    h_g4_rel_err_3s->GetXaxis()->SetTitleOffset(1.1);
    h_g4_rel_err_3s->GetXaxis()->SetLabelSize(0.1153);
    h_g4_rel_err_3s->GetXaxis()->SetLabelOffset(0.02);
    h_g4_rel_err_3s->GetXaxis()->SetTickLength(0.07);

    h_g4_rel_err_3s->GetYaxis()->SetTitle("Rel. Diff. (%)");
    h_g4_rel_err_3s->GetYaxis()->CenterTitle();
    h_g4_rel_err_3s->GetYaxis()->SetTitleSize(0.131);
    h_g4_rel_err_3s->GetYaxis()->SetTitleOffset(0.415);
    h_g4_rel_err_3s->GetYaxis()->SetLabelSize(0.116);
    h_g4_rel_err_3s->GetYaxis()->SetLabelOffset(0.008);
    h_g4_rel_err_3s->GetYaxis()->SetTickLength(0.04);
    h_g4_rel_err_3s->GetYaxis()->SetNdivisions(503);
    // h_g4_rel_err->GetYaxis()->SetRangeUser(-55, 55);

    h_g4_rel_err_3s->SetLineColorAlpha(kGray, 0.7);
    h_g4_rel_err_3s->SetFillColorAlpha(kGray, 0.7);
    h_g4_rel_err_3s->SetMarkerSize(0);
    h_g4_rel_err->SetLineColorAlpha(kGray + 1, 0.7);
    h_g4_rel_err->SetFillColorAlpha(kGray + 1, 0.7);
    h_g4_rel_err->SetMarkerSize(0);

    h_rel_diff->SetLineColor(celeritas_color);

    // Draw stat. err. and rel. diff. histograms
    h_g4_rel_err_3s->Draw("hist E2");
    h_g4_rel_err->Draw("hist E2 sames");
    h_rel_diff->Draw("hist sames");

    pad_bottom->RedrawAxis();
}
