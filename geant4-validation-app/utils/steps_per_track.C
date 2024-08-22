//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file steps_per_track.C
//! \brief Plot Geant4 vs. Celeritas steps per track and their relative error.
//---------------------------------------------------------------------------//
#include <iostream>
#include <TCanvas.h>
#include <TH1D.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TStyle.h>
#include <TText.h>

//---------------------------------------------------------------------------//
/*!
 * Enums for safely selecting particles and MC codes.
 */
enum MC
{
    g4,
    cel,
    size
};

enum PID
{
    e_plus,
    e_minus,
    photon,
    size
};

//---------------------------------------------------------------------------//
/*!
 * Geant4 vs. Celeritas steps per track for electrons, positrons, gammas,
 * and their relative errors.
 *
 * Make sure you have celeritas standard plotting options loaded into ROOT.
 * Instructions in celeritas-docs/utils/rootlogon.C
 *
 * Usage:
 * $ root steps_per_track.C
 *
 * The G4 bin data in this plot is extracted using
 * g4-validation-app/utils/diagnostics.C .
 *
 * Results come from cms-demo-loop/input/medium.hepmc3
 *
 * \note
 * The data starts at bin[1], with bin[0] being reserved for ROOT's underflow:
 * - bin[0] = underflow, manually set to 0.
 * - bin[1] = number of tracks with 1 step.
 */
void steps_per_track()
{
    // Create histograms pointers for G4 and Celeritas for each particle type
    TH1D* h_steps[MC::size][PID::size];
    static int const n_bins = 180;

    for (int i = 0; i < MC::size; i++)
    {
        for (int j = 0; j < PID::size; j++)
        {
            // Initialize pointers
            h_steps[i][j] = new TH1D("", "", n_bins, 0, n_bins);
        }
    }

    // Histrograms for the relative error
    TH1D* h_error[PID::size];

    for (int j = 0; j < PID::size; j++)
    {
        // Initialize pointers
        h_error[j] = new TH1D("", "", n_bins, 0, n_bins);
    }

    // >>> Geant4 histogram data
    int const g4_positron_steps[200] = {
        0,     3177,  53962, 25350, 19264, 16336, 14681, 12984, 12419, 12401,
        12423, 12616, 13058, 13265, 13421, 13516, 12573, 11987, 11400, 10855,
        10148, 9521,  8927,  8231,  7937,  7377,  6898,  6486,  6035,  5592,
        5263,  4848,  4487,  4143,  3767,  3553,  3360,  3176,  2927,  2749,
        2496,  2352,  2137,  2011,  1910,  1776,  1622,  1480,  1318,  1309,
        1240,  1150,  1063,  932,   843,   816,   749,   739,   649,   623,
        595,   528,   522,   495,   432,   397,   400,   376,   342,   266,
        260,   268,   253,   235,   212,   166,   178,   153,   153,   156,
        116,   121,   143,   102,   91,    94,    75,    91,    69,    54,
        60,    55,    61,    54,    48,    51,    42,    41,    33,    22,
        16,    24,    16,    22,    24,    28,    29,    18,    13,    13,
        12,    9,     15,    5,     9,     9,     8,     5,     8,     8,
        6,     4,     5,     6,     5,     4,     5,     0,     3,     2,
        3,     0,     1,     0,     3,     1,     0,     1,     0,     0,
        0,     0,     0,     1,     1,     0,     0,     0,     0,     2,
        0,     1,     0,     0,     0,     1,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0};

    int const g4_electron_steps[200]
        = {0,     6080292, 512384, 215510, 141365, 103657, 81385, 68801, 61376,
           56837, 53848,   51677,  49907,  48110,  46322,  44080, 41427, 38560,
           35719, 33012,   30550,  27805,  25719,  23779,  21846, 19804, 17891,
           16720, 15584,   14568,  13077,  11994,  11079,  10387, 9453,  8788,
           8272,  7626,    7012,   6494,   6120,   5675,   5158,  4785,  4444,
           4193,  3754,    3562,   3188,   3129,   2903,   2582,  2548,  2300,
           2253,  2034,    1919,   1761,   1707,   1565,   1478,  1324,  1264,
           1232,  1070,    1057,   969,    877,    807,    782,   712,   706,
           715,   576,     593,    539,    512,    501,    466,   434,   414,
           340,   379,     283,    348,    330,    306,    283,   258,   228,
           220,   201,     205,    204,    180,    152,    141,   125,   146,
           120,   136,     96,     105,    91,     83,     91,    75,    82,
           70,    61,      71,     77,     48,     54,     43,    44,    56,
           44,    35,      46,     29,     32,     24,     25,    27,    13,
           17,    22,      19,     17,     14,     14,     14,    11,    12,
           7,     5,       7,      5,      10,     5,      8,     3,     4,
           6,     6,       2,      4,      5,      1,      7,     2,     6,
           2,     1,       1,      1,      4,      3,      0,     1,     0,
           0,     3,       2,      3,      1,      1,      2,     0,     0,
           0,     0,       1,      0,      0,      0,      1,     0,     0,
           0,     0,       1,      0,      0,      0,      0,     0,     0,
           0,     0,       0,      0,      0,      1,      0,     0,     0,
           0,     0};

    int const g4_gamma_steps[200]
        = {0,      1080174, 400325, 448511, 270035, 245198, 183655, 177353,
           135274, 135067,  102510, 104504, 80527,  83630,  64295,  68016,
           52581,  56173,   43632,  47328,  36316,  40481,  30885,  34621,
           26607,  29857,   23156,  25917,  20098,  22834,  17661,  19805,
           15451,  17813,   13609,  15763,  12329,  14004,  10819,  12407,
           9782,   10957,   8569,   9755,   7651,   8684,   7055,   7839,
           6322,   7048,    5756,   6471,   5152,   5781,   4580,   5345,
           4238,   4801,    3804,   4278,   3469,   3873,   3244,   3550,
           2872,   3278,    2560,   2877,   2356,   2606,   2121,   2454,
           2007,   2108,    1823,   2024,   1538,   1706,   1411,   1594,
           1374,   1444,    1140,   1299,   1035,   1090,   875,    946,
           754,    828,     645,    621,    526,    560,    469,    392,
           340,    316,     238,    217,    168,    142,    104,    72,
           27,     12,      8,      3,      5,      2,      0,      0,
           1,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0};

    // >>> Celeritas histogram data
    int const cel_positron_steps[200] = {
        0,     24621, 42058, 22206, 18238, 15773, 14171, 13126, 12273, 12354,
        12361, 12802, 13308, 13686, 13648, 13341, 13158, 12308, 11654, 10872,
        10192, 9458,  9009,  8466,  7619,  7333,  6763,  6271,  5876,  5385,
        4936,  4695,  4289,  3955,  3643,  3383,  3136,  2942,  2710,  2505,
        2345,  2165,  2094,  1827,  1725,  1621,  1443,  1327,  1234,  1202,
        1065,  976,   958,   927,   770,   735,   675,   646,   655,   541,
        476,   514,   429,   426,   402,   339,   330,   296,   284,   226,
        213,   189,   220,   184,   167,   167,   160,   124,   135,   109,
        98,    90,    102,   95,    81,    71,    75,    61,    69,    50,
        43,    41,    48,    42,    29,    28,    35,    34,    24,    17,
        21,    15,    21,    18,    16,    11,    14,    11,    10,    14,
        7,     11,    3,     11,    3,     4,     3,     3,     6,     3,
        4,     5,     3,     4,     6,     1,     3,     1,     1,     5,
        1,     1,     2,     0,     0,     0,     1,     2,     1,     0,
        0,     0,     1,     0,     0,     0,     0,     1,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        1,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     1,     0,
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0};

    int const cel_electron_steps[200]
        = {0,     6030882, 504255, 214347, 143832, 105377, 82455, 68996, 62107,
           57289, 54278,   52305,  51331,  49899,  47061,  44898, 42034, 38919,
           35858, 33171,   30887,  28153,  25802,  23560,  21493, 19875, 18099,
           16515, 15481,   13902,  12703,  12000,  10975,  10093, 9352,  8547,
           7634,  7239,    6742,   6205,   5835,   5414,   4932,  4693,  4322,
           3959,  3630,    3400,   3200,   2925,   2694,   2446,  2335,  2191,
           2042,  1909,    1745,   1643,   1613,   1553,   1307,  1294,  1177,
           1134,  1029,    980,    893,    876,    870,    755,   743,   655,
           589,   561,     497,    529,    500,    457,    438,   401,   415,
           372,   329,     323,    321,    290,    281,    207,   235,   230,
           221,   195,     174,    162,    164,    153,    147,   131,   138,
           128,   112,     109,    98,     96,     90,     77,    65,    78,
           70,    61,      68,     57,     43,     47,     45,    37,    38,
           30,    31,      32,     23,     35,     24,     25,    30,    26,
           20,    15,      19,     13,     8,      15,     16,    11,    11,
           4,     8,       6,      10,     5,      6,      7,     4,     6,
           7,     6,       2,      4,      6,      2,      5,     3,     3,
           1,     3,       1,      0,      2,      0,      1,     1,     4,
           0,     0,       1,      0,      0,      1,      1,     0,     0,
           2,     1,       0,      0,      0,      0,      0,     1,     0,
           0,     0,       1,      0,      0,      0,      0,     0,     1,
           0,     0,       0,      0,      0,      1,      0,     0,     0,
           0,     0};

    int const cel_gamma_steps[200]
        = {0,      1074306, 395556, 451711, 268020, 245149, 182634, 177100,
           133821, 133688,  102164, 104178, 79954,  83255,  63897,  68172,
           52121,  56733,   42831,  47553,  36223,  40889,  31152,  34769,
           26249,  29878,   22934,  26036,  20280,  22847,  17866,  20315,
           15668,  17866,   13789,  15927,  12417,  14232,  11169,  12772,
           9740,   11083,   8876,   10060,  8090,   9244,   7281,   8151,
           6552,   7461,    5839,   6716,   5263,   6093,   4849,   5520,
           4435,   4938,    3971,   4543,   3757,   4074,   3321,   3810,
           3073,   3409,    2738,   3040,   2426,   2781,   2243,   2469,
           2078,   2257,    1922,   1998,   1696,   1924,   1574,   1704,
           1386,   1509,    1232,   1377,   1049,   1160,   839,    1002,
           805,    863,     659,    707,    583,    598,    461,    452,
           343,    297,     218,    232,    181,    160,    123,    56,
           47,     16,      9,      2,      0,      4,      0,      0,
           0,      1,       0,      0,      0,      1,      0,      1,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0,
           0,      0,       0,      0,      0,      0,      0,      0};

    // >>> Fill histogram data
    auto& h_steps_g4 = h_steps[MC::g4];
    auto& h_steps_cel = h_steps[MC::cel];

    for (int i = 0; i < h_steps_g4[photon]->GetNbinsX(); i++)
    {
        // Geant4
        h_steps_g4[e_plus]->SetBinContent(i, g4_positron_steps[i]);
        h_steps_g4[e_minus]->SetBinContent(i, g4_electron_steps[i]);
        h_steps_g4[photon]->SetBinContent(i, g4_gamma_steps[i]);

        // Celeritas
        h_steps_cel[e_plus]->SetBinContent(i, cel_positron_steps[i]);
        h_steps_cel[e_minus]->SetBinContent(i, cel_electron_steps[i]);
        h_steps_cel[photon]->SetBinContent(i, cel_gamma_steps[i]);
    }

    // Calculate relative error
    for (int i = 0; i < PID::size; i++)
    {
        h_error[i] = (TH1D*)h_steps_cel[i]->Clone();
        h_error[i]->Add(h_steps_g4[i], -1);
        h_error[i]->Divide(h_steps_cel[i]);
    }

    // Create clones to draw lines over the shaded error bar regions
    TH1D* h_steps_clones[PID::size];
    for (int j = 0; j < PID::size; j++)
    {
        h_steps_clones[j] = (TH1D*)h_steps_cel[j]->Clone();
    }

    h_steps_clones[e_plus]->SetLineColor(kGreen + 2);
    h_steps_clones[e_minus]->SetLineColor(kBlue);
    h_steps_clones[photon]->SetLineColor(kViolet);

    // Create canvas
    auto canvas = new TCanvas("", "", 900, 800);
    canvas->Divide(1, 2);

    // Create top pad
    TPad* pad1 = new TPad("pad1", "", 0.0, 0.3, 1.0, 1.0);
    pad1->SetBottomMargin(0.02);
    pad1->Draw();
    pad1->cd();
    pad1->SetLogy();

    // Set histogram attributes
    for (int i = 0; i < MC::size; i++)
    {
        h_steps[i][e_plus]->SetLineColor(kGreen + 2);
        h_steps[i][e_minus]->SetLineColor(kBlue);
        h_steps[i][photon]->SetLineColor(kViolet);

        h_steps[i][e_plus]->SetFillColorAlpha(kGreen + 2, 0.3);
        h_steps[i][e_minus]->SetFillColorAlpha(kBlue, 0.3);
        h_steps[i][photon]->SetFillColorAlpha(kViolet, 0.3);

        h_steps[i][e_plus]->SetMarkerColor(kGreen + 2);
        h_steps[i][e_minus]->SetMarkerColor(kBlue);
        h_steps[i][photon]->SetMarkerColor(kViolet);
    }

    for (int j = 0; j < PID::size; j++)
    {
        h_steps[MC::g4][j]->SetMarkerSize(1.1);
        h_steps[MC::cel][j]->SetMarkerSize(0);
    }

    // Make X marker a little bit bigger
    h_steps[MC::g4][e_minus]->SetMarkerSize(1.3);

    h_steps[MC::g4][e_plus]->SetMarkerStyle(53);
    h_steps[MC::g4][e_minus]->SetMarkerStyle(52);
    h_steps[MC::g4][photon]->SetMarkerStyle(55);

    for (int j = 0; j < PID::size; j++)
    {
        h_steps[MC::g4][j]->SetLineWidth(2);
        h_steps[MC::cel][j]->SetLineWidth(2);

        h_steps_clones[j]->SetLineWidth(2);
    }

    h_steps_g4[e_minus]->GetXaxis()->SetLabelOffset(99);
    h_steps_g4[e_minus]->GetYaxis()->SetRangeUser(0.5, 1E7);
    h_steps_g4[e_minus]->GetYaxis()->SetNdivisions(-5);

    // Draw plots
    h_steps_g4[e_minus]->Draw("P");
    h_steps_g4[e_plus]->Draw("P sames");
    h_steps_g4[photon]->Draw("P sames");

    h_steps_cel[e_minus]->Draw("E2 sames");
    h_steps_cel[e_plus]->Draw("E2 sames");
    h_steps_cel[photon]->Draw("E2 sames");

    for (int j = 0; j < PID::size; j++)
    {
        h_steps_clones[j]->Draw("sames");
    }

    // Draw legends
    auto legend_g4 = new TLegend(0.57, 0.65, 0.70, 0.86);
    legend_g4->SetHeader("Geant4");
    legend_g4->AddEntry(h_steps_g4[e_minus], "e^{-}", "p");
    legend_g4->AddEntry(h_steps_g4[e_plus], "e^{+}", "p");
    legend_g4->AddEntry(h_steps_g4[photon], "#gamma", "p");
    legend_g4->SetMargin(0.65);
    legend_g4->SetLineColor(kGray);
    // legend_g4->Draw();

    auto legend_cel = new TLegend(0.72, 0.65, 0.86, 0.86);
    legend_cel->SetHeader("Celeritas*");
    legend_cel->AddEntry(h_steps_cel[e_minus], "e^{-}", "lf");
    legend_cel->AddEntry(h_steps_cel[e_plus], "e^{+}", "lf");
    legend_cel->AddEntry(h_steps_cel[photon], "#gamma", "lf");
    legend_cel->SetMargin(0.65);
    legend_cel->SetLineColor(kGray);
    // legend_cel->Draw();

    auto legend_g42 = new TLegend(0.36, 0.73, 0.57, 0.86);
    legend_g42->SetHeader(" Geant4 v11.0.3");
    legend_g42->SetNColumns(3);
    legend_g42->AddEntry(h_steps_g4[e_minus], "e^{-}", "p");
    legend_g42->AddEntry(h_steps_g4[e_plus], "e^{+}", "p");
    legend_g42->AddEntry(h_steps_g4[photon], "#gamma^{ }", "p");
    legend_g42->SetMargin(0.37);
    legend_g42->SetLineColor(kGray);
    legend_g42->Draw();

    auto legend_cel2 = new TLegend(0.58, 0.73, 0.865, 0.86);
    legend_cel2->SetHeader("Celeritas (c0a251de4)");
    legend_cel2->SetNColumns(3);
    legend_cel2->AddEntry(h_steps_cel[e_minus], "e^{-}", "lf");
    legend_cel2->AddEntry(h_steps_cel[e_plus], "e^{+}", "lf");
    legend_cel2->AddEntry(h_steps_cel[photon], "#gamma^{ }", "lf");
    legend_cel2->SetMargin(0.65);
    legend_cel2->SetLineColor(kGray);
    legend_cel2->Draw();

    // Draw title
    auto title_text
        = new TText(0.11, 0.92, "Steps per track per particle type");
    title_text->SetNDC();
    title_text->SetTextColor(kGray);
    title_text->Draw();

    // Redraw axis, so the tick marks stay in front of the histogram lines
    pad1->RedrawAxis();

    // Move back to top canvas
    canvas->cd();

    // Create bottom pad
    TPad* pad2 = new TPad("pad2", "", 0.0, 0.0, 1.0, 0.3);
    pad2->SetTopMargin(0.01);
    pad2->SetBottomMargin(0.4);
    pad2->Draw();
    pad2->cd();

    h_error[e_minus]->GetXaxis()->SetTitle("Steps per track");
    h_error[e_minus]->GetXaxis()->CenterTitle();
    h_error[e_minus]->GetXaxis()->SetTitleSize(0.15);
    h_error[e_minus]->GetXaxis()->SetLabelSize(0.1155);
    h_error[e_minus]->GetXaxis()->SetLabelOffset(0.04);
    h_error[e_minus]->GetXaxis()->SetTitleOffset(1.2);

    h_error[e_minus]->GetYaxis()->SetTitle("Rel. Diff.");
    h_error[e_minus]->GetYaxis()->CenterTitle();
    h_error[e_minus]->GetYaxis()->SetTitleSize(0.13);
    h_error[e_minus]->GetYaxis()->SetTitleOffset(0.37);
    h_error[e_minus]->GetYaxis()->SetLabelSize(0.117);
    h_error[e_minus]->GetYaxis()->SetLabelOffset(0.011);
    h_error[e_minus]->GetYaxis()->SetRangeUser(-2.5, 1.5);

    h_error[e_minus]->SetLineColor(kBlue);
    h_error[e_plus]->SetLineColor(kGreen + 2);
    h_error[photon]->SetLineColor(kViolet);

    for (int j = 0; j < PID::size; j++)
    {
        h_error[j]->SetLineWidth(1);
    }

    h_error[e_minus]->Draw();
    h_error[e_plus]->Draw("sames");
    h_error[photon]->Draw("sames");

    // Draw line at 0
    auto line = new TLine(0, 0, 125, 0);
    line->SetLineColor(kBlack);
    line->SetLineWidth(2);
    line->SetLineStyle(7);
    line->Draw();

    // Refresh axis
    pad2->RedrawAxis();

    // canvas->SaveAs("steps-per-track.pdf");
}
