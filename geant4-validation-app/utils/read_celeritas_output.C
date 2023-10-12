//----------------------------------*-C++-*----------------------------------//
// Copyright 2020-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file read_celeritas_output.C
//! \brief Example macro for looping over a Celeritas MC Truth output.
//---------------------------------------------------------------------------//
#include <iostream>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>

void hello()
{
    std::cout << "hello" << std::endl;
}

//---------------------------------------------------------------------------//
/*!
 * Example macro that loops over the steps tree and prints up to 2 steps.
 *
 * This example prints only a few variables. The easiest way to view the full
 * list of leaves is to invoke `TTree:Show(0)`, which will show entry 0 of the
 * steps tree:
 * $ root mctrutht.root
 * root [0] steps->Show(0)
 *
 * Usage:
 * root[0] .x read_celeritas_output.C("path/to/celeritas_mctruth.root")
 */
void read_celeritas_output(const std::string file_name)
{
    using std::cout;
    using std::endl;

    auto input      = new TFile(file_name.c_str(), "read");
    auto steps_tree = (TTree*)input->Get("steps");

    int num_steps = std::min((long)steps_tree->GetEntries(), 2l);

    for (int i = 0; i < num_steps; i++)
    {
        steps_tree->GetEntry(i);

        cout << "--- STEP " << i << endl;
        cout << "event id: " << steps_tree->GetLeaf("event_id")->GetValue()
             << endl;
        cout << "track id: " << steps_tree->GetLeaf("track_id")->GetValue()
             << endl;
        cout << "step length (cm): "
             << steps_tree->GetLeaf("step_length")->GetValue() << endl;

        cout << "dir (pre): ";
        for (int j = 0; j < 3; j++)
        {
            cout << steps_tree->GetLeaf("pre_dir")->GetValue(j) << " ";
        }
        cout << endl;
        cout << endl;
    }

    input->Close();
}
