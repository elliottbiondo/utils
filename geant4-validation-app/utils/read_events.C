//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file read_events.C
//! \brief Example macro for looping over event tree.
//---------------------------------------------------------------------------//
#include "../src/RootData.hh"

#include <iostream>
#include <algorithm>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>
#include <TSystem.h>

//---------------------------------------------------------------------------//
/*!
 * Example macro that loops over the event tree and prints up to 2 events.
 * Only a couple secondaries are printed to avoid cluttering.
 *
 * Usage:
 * root[0] .x read_events.C("path/to/g4_output.root")
 */
void read_events(const std::string file_name)
{
    using std::cout;
    using std::endl;

    // Load rootdata shared library
    const char* librootdata = "../build/librootdata";
    gSystem->Load(librootdata);

    TFile* input      = new TFile(file_name.c_str(), "read");
    TTree* event_tree = (TTree*)input->Get("events");

    rootdata::Event* event = nullptr;
    event_tree->SetBranchAddress("event", &event);

    int number_of_events = std::min((long)event_tree->GetEntries(), 2l);

    for (int i = 0; i < number_of_events; i++)
    {
        event_tree->GetEntry(i);

        cout << ">>> Event " << event->id << endl;
        cout << "-----------" << endl;

        // Primaries
        const auto primaries = event->primaries;
        cout << " Primaries (" << primaries.size() << ")" << endl;

        const auto primary  = primaries.at(0);
        const auto prim_pos = primary.vertex_position;
        const auto prim_dir = primary.vertex_direction;

        cout << "  | pdg encoding           : " << primary.pdg << endl;
        cout << "  | vertex position [cm]   : " << prim_pos.x << ", "
             << prim_pos.y << ", " << prim_pos.z << endl;
        cout << "  | vertex direction       : " << prim_dir.x << ", "
             << prim_dir.y << ", " << prim_dir.z << endl;
        cout << "  | vertex energy [MeV]    : " << primary.vertex_energy
             << endl;
        cout << "  | track length [cm]      : " << primary.length << endl;
        cout << "  | energy deposition [MeV]: " << primary.energy_dep << endl;
        cout << "  | number of steps        : " << primary.number_of_steps
             << endl;
        cout << "  | steps (" << primary.steps.size() << ")" << endl;

        if (primary.steps.size() > 0)
        {
            rootdata::Step prim_step = primary.steps.at(0);
            cout << "  |  | process          : "
                 << rootdata::to_process_name(prim_step.process_id) << endl;
            cout << "  |  | energy [MeV]     : " << prim_step.kinetic_energy
                 << endl;
            cout << "  |  | energy loss [MeV]: " << prim_step.energy_loss
                 << endl;
            cout << "  |  | position [cm]    : " << prim_step.position.x
                 << ", " << prim_step.position.y << ", "
                 << prim_step.position.z << endl;
            cout << "  |  | direction        : " << prim_step.direction.x
                 << ", " << prim_step.direction.y << ", "
                 << prim_step.direction.z << endl;
            if (primary.steps.size() > 1)
            {
                cout << "  |  | --" << endl;
                cout << "  |  :" << endl;
                cout << "  |  :" << endl;
            }
        }

        // Secondaries
        const auto secondaries = event->secondaries;
        cout << "  | Secondaries (" << secondaries.size() << ")" << endl;

        int  number_of_secondaries = std::min((long)secondaries.size(), 2l);
        bool ellipsis = (number_of_secondaries > 2) ? true : false;

        for (int j = 0; j < number_of_secondaries; j++)
        {
            const auto secondary = secondaries.at(j);
            const auto pos       = secondary.vertex_position;
            const auto dir       = secondary.vertex_direction;

            cout << "     | pdg encoding           : " << secondary.pdg << endl;
            cout << "     | vertex position [cm]   : " << pos.x << ", "
                 << pos.y << ", " << pos.z << endl;
            cout << "     | vertex direction       : " << dir.x << ", "
                 << dir.y << ", " << dir.z << endl;
            cout << "     | vertex energy [MeV]    : "
                 << secondary.vertex_energy << endl;
            cout << "     | track length [cm]      : " << secondary.length
                 << endl;
            cout << "     | energy deposition [MeV]: " << secondary.energy_dep
                 << endl;
            cout << "     | number of steps        : "
                 << secondary.number_of_steps << endl;
            cout << "     | steps (" << secondary.steps.size() << ")" << endl;

            if (secondary.steps.size() > 0)
            {
                rootdata::Step step = secondary.steps.at(0);

                cout << "     |  | process          : "
                     << rootdata::to_process_name(step.process_id) << endl;
                cout << "     |  | energy [MeV]     : " << step.kinetic_energy
                     << endl;
                cout << "     |  | energy loss [MeV]: "
                     << secondary.steps.at(0).energy_loss << endl;
                cout << "     |  | position [cm]    : " << step.position.x
                     << ", " << step.position.y << ", " << step.position.z
                     << endl;
                cout << "     |  | direction        : " << step.direction.x
                     << ", " << step.direction.y << ", " << step.direction.z
                     << endl;

                if (secondary.steps.size() > 1)
                {
                    cout << "     |  | --" << endl;
                    cout << "     |  :" << endl;
                    cout << "     |  :" << endl;
                }
            }
            if (number_of_secondaries > 1)
            {
                cout << "     | --" << endl;
            }
            if (j == number_of_secondaries - 1)
            {
                if (ellipsis)
                {
                    cout << "     :" << endl;
                    cout << "     :" << endl;
                }
            }
        }
        cout << endl;
    }
    input->Close();
}
