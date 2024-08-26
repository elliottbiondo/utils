//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file HepMC3Reader.cc
//---------------------------------------------------------------------------//
#include "HepMC3Reader.hh"

#include <iostream>
#include <G4SystemOfUnits.hh>
#include <HepMC3/ReaderFactory.h>
#include <assert.h>

#include "JsonReader.hh"

//---------------------------------------------------------------------------//
/*!
 * Singleton declaration.
 */
static HepMC3Reader* hepmc3_singleton = nullptr;

//---------------------------------------------------------------------------//
// PUBLIC
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Constructor singleton.
 */
void HepMC3Reader::construct()
{
    if (!hepmc3_singleton)
    {
        hepmc3_singleton = new HepMC3Reader();
    }
    else
    {
        std::cout << "HepMC3 Reader already constructed. Nothing to do.\n";
    }
}

//---------------------------------------------------------------------------//
/*!
 * Get static HepMC3Reader instance. \c construct() *MUST* be called before
 * this.
 */
HepMC3Reader* HepMC3Reader::instance()
{
    return hepmc3_singleton;
}

//---------------------------------------------------------------------------//
/*!
 * Read the next event of the file and store its primaries.
 */
bool HepMC3Reader::read_event()
{
    if (input_file_->read_event(gen_event_))
    {
        assert(gen_event_.momentum_unit() != HepMC3::Units::MEV
               && gen_event_.length_unit() != HepMC3::Units::CM);

        event_primaries_.clear();

        auto const& pos = gen_event_.event_pos();
        auto const& particles = gen_event_.particles();

        for (auto const& particle : particles)
        {
            auto const& data = particle->data();
            auto const& p = data.momentum;

            Primary primary;
            primary.pdg = data.pid;
            primary.energy = data.momentum.e();
            primary.momentum[0] = p.x();
            primary.momentum[1] = p.y();
            primary.momentum[2] = p.z();
            primary.vertex[0] = pos.x() * cm;
            primary.vertex[1] = pos.y() * cm;
            primary.vertex[2] = pos.z() * cm;

            event_primaries_.push_back(std::move(primary));
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * Construct new HepMC3Reader from json input data.
 *
 * \note
 * \c number_of_events_ starts at -1 due to the while loop, which only fails
 * after an extra count.
 */
HepMC3Reader::HepMC3Reader() : number_of_events_(-1)
{
    // Load input
    auto const json = JsonReader::instance()->json();
    auto const input = json.at("simulation").at("hepmc3").get<std::string>();
    input_file_ = HepMC3::deduce_reader(input);

    // Fetch total number of events
    auto const file = HepMC3::deduce_reader(input);

    while (!file->failed())
    {
        // Parse the next event from the record
        HepMC3::GenEvent gen_event;
        file->read_event(gen_event);
        number_of_events_++;
    }
}
