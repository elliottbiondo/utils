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
static std::string sd_dir = "world_sd_0_0/";
static std::string hist_name = "energy";
// Axis, title, and legend
static char const* hist_title = "";
static char const* commit_hash = "*a075958fc";
static char const* x_axis_title = "Step energy deposition [MeV]";
static char const* geant4_legend = "Geant4 v11.3.0";
static char const* celeritas_legend = "Celeritas v0.6 dev*";

//---------------------------------------------------------------------------//
// Compare Celeritas vs. Geant4 TH1D histograms.
void th1d_compare()
{
    auto file_g4 = TFile::Open("output-g4.root", "read");
    auto file_cel = TFile::Open("output-cel.root", "read");

    std::string const hist_dir = "histograms/";
    std::string sd_hist = hist_dir + sd_dir + hist_name;
    auto h_g4 = file_g4->Get<TH1D>(sd_hist.c_str());
    auto h_cel = file_cel->Get<TH1D>(sd_hist.c_str());
    h_g4->SetTitle("");
    h_cel->SetTitle("");

    // Initialize histograms with correct binning
    auto const nbins = h_g4->GetNbinsX();
    auto const xmin = h_g4->GetXaxis()->GetXmin();
    auto const xmax = h_g4->GetXaxis()->GetXmax();

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
    h_g4->GetYaxis()->SetLabelSize(0.05);
    h_g4->GetYaxis()->CenterTitle();

    // Draw histograms
    h_g4->Draw("PE2");
    h_cel->Draw("hist sames");

    auto legend_top = new TLegend(0.55, 0.46, 0.86, 0.86);
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
