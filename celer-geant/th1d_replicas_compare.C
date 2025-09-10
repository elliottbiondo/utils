//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
#include <iostream>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TText.h>

//---------------------------------------------------------------------------//
// Sensitive detector and histogram names
static std::string sd_name = "sd_gap";  //< TestEM3 example with 50 replicas
static std::string hist_name = "energy_dep";
// Axis, title, and legend
static char const* hist_title = "Step energy deposition";
static char const* commit_hash = "[commit hash]";
static char const* x_axis_title = "X-axis position [cm]";
static char const* geant4_legend = "Geant4 v11.3.0";
static char const* celeritas_legend = "Celeritas v0.6 dev";

//---------------------------------------------------------------------------//
//! Example loop for the TestEM3 geometry.
void th1d_replicas_compare()
{
    auto file_g4 = TFile::Open("output-g4.root", "read");
    auto file_cel = TFile::Open("output-cel.root", "read");

    std::string const hist_dir = "histograms/";

    // Init global histograms and binning after loading the first SD histogram
    TH1D *h_g4, *h_cel;
    size_t nbins;
    double xmin, xmax;

    // TestEM3 has 50 layers of gaps + absorbers pairs
    // Here we are only accumulating data from sd_gap
    for (int i = 0; i < 50; i++)
    {
        // Instance ID grows by PV: gap (0) | abs (1) | gap (2) | abs (3) | ...
        std::string instance_id = std::to_string(2 * i);  // sd_gaps are even
        // Copy number grows linearly with each SD (0-49) for both gap and abs
        std::string copy_num = std::to_string(i);
        // SD directory is: sd_name_[instance_id]_[copy_num]/
        std::string sd_dir = sd_name + "_" + instance_id + "_" + copy_num + "/";

        // Full path to the histogram
        std::string full_path = hist_dir + sd_dir + hist_name;

        auto this_hg4 = file_g4->Get<TH1D>(full_path.c_str());
        auto this_hcel = file_cel->Get<TH1D>(full_path.c_str());

        if (i == 0)
        {
            // Initialize histograms with correct binning
            nbins = this_hg4->GetNbinsX();
            xmin = this_hg4->GetXaxis()->GetXmin();
            xmax = this_hg4->GetXaxis()->GetXmax();
            h_g4 = new TH1D("g4", "", nbins, xmin, xmax);
            h_cel = new TH1D("cel", "", nbins, xmin, xmax);
        }

        // Add this histogram to the total one
        h_g4->Add(this_hg4);
        h_cel->Add(this_hcel);
    }

    // Create relative error histograms
    auto h_g4_rel_err = new TH1D("G4 rel. err.", "", nbins, xmin, xmax);
    auto h_g4_rel_err_3s
        = new TH1D("G4 rel. err. 3sigma", "", nbins, xmin, xmax);
    for (int i = 0; i < nbins; i++)
    {
        double error = h_g4->GetBinError(i);
        double value = h_g4->GetBinContent(i);
        double rel_err = value ? error / value : 0;

        h_g4_rel_err->SetBinContent(i, 0);
        h_g4_rel_err->SetBinError(i, rel_err * 100);
        h_g4_rel_err_3s->SetBinContent(i, 0);
        h_g4_rel_err_3s->SetBinError(i, 3 * rel_err * 100);
    }

    // Create relative difference histogram: (Celeritas - Geant4) / Celeritas
    auto h_rel_diff = (TH1D*)h_cel->Clone();
    h_rel_diff->Add(h_g4, -1);
    h_rel_diff->Divide(h_cel);
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
