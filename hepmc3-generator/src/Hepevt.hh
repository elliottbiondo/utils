//----------------------------------*-C++-*----------------------------------//
// Copyright 2020 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Hepevt.hh
//---------------------------------------------------------------------------//
#pragma once

namespace hepmc3gen
{
//---------------------------------------------------------------------------//
/*!
 * Store hepevt ascii data.
 *
 * \note
 * CMS Pythia files have a shorter structure than full HEPEVT files.
 * This same shorter structure is used in Geant4 by the G4HEPEvtInterface.
 * Below the full HEPEVT format is shown, with 'x' marks showing which elements
 * are stored/read by Pythia/Geant4.
 *
 * [ ] "E" [ ] eventNumber [x] numberOfParticles
 *  status PDG mother1 mother2 daughter2 daughter2 Px Py Pz E m x y z t
 * [  x     x                      x         x     x  x  x    x        ]
 *
 * HepMC3 ReaderHEPEVT reads partial/full HEPEVT files. The "E" at the
 * beginning of the event line definition seems to be mandatory in HepMC3,
 * though it is not in other HEPEVT readers I've encoutered in the past.
 */
struct HepevtHeader
{
    int          event_number;
    unsigned int number_of_particles;
};

struct HepevtParticle
{
    // The struct declaration ordering is the hepevt ascii standard
    int    status;
    int    pdg;
    int    mother_1;
    int    mother_2;
    int    daughter_1;
    int    daughter_2;
    double p_x;
    double p_y;
    double p_z;
    double energy;
    double mass;
    double x;
    double y;
    double z;
    double time;
};

//---------------------------------------------------------------------------//
} // namespace hepmc3gen
