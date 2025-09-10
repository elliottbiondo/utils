//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
#include <TCanvas.h>
#include <TFile.h>
#include <TH2D.h>
#include <TPaveStats.h>
#include <TStyle.h>
#include <TText.h>

//---------------------------------------------------------------------------//
// Sensitive detector and histogram names
static std::string sd_dir = "world_sd_0_0/";
static std::string hist_name = "pos_xy";
// Title
static char const* hist_title = "Celeritas 0.6 dev [a075958fc]";

//---------------------------------------------------------------------------//
// Plot TH2D histogram.
void th2d_plot()
{
    auto file = TFile::Open("output.root", "read");

    std::string full_path = "histograms/" + sd_dir + hist_name;
    auto* h2d = file->Get<TH2D>(full_path.c_str());

    auto c = new TCanvas("", "", 700, 600);
    c->SetLogz();
    c->SetRightMargin(0.13);
    h2d->SetTitle("");
    h2d->GetXaxis()->SetTitle("Step position x [cm]");
    h2d->GetXaxis()->SetTitleOffset(1.2);
    h2d->GetXaxis()->CenterTitle();
    h2d->GetYaxis()->SetTitle("Step position y [cm]");
    h2d->GetYaxis()->CenterTitle();
    h2d->Draw("ncolz");

    gStyle->SetOptStat("emr");  // Include (e)ntries, (m)ean, and (r)ms
    c->Update();  // Update stats box ptr after ::Draw

    // Update stats box
    h2d->SetStats(true);
    auto stats = dynamic_cast<TPaveStats*>(h2d->FindObject("stats"));
    stats->SetBorderSize(1);  // Remove shadow
    // Move box
    stats->SetX1NDC(0.63);
    stats->SetX2NDC(0.83);
    stats->SetY1NDC(0.65);
    stats->SetY2NDC(0.85);
    c->Modified();  // Redraw canvas

    // Add title
    auto title_text = new TText(0.17, 0.92, hist_title);
    title_text->SetNDC();
    title_text->SetTextColor(kGray);
    title_text->Draw();
}
