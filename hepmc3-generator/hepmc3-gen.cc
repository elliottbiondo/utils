//----------------------------------*-C++-*----------------------------------//
// Copyright 2020 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file hepmc3-gen.cc
//---------------------------------------------------------------------------//
#include <iostream>
#include <fstream>
#include <strstream>
#include <math.h>
#include <random>

#include <HepMC3/GenEvent.h>
#include <HepMC3/GenParticle.h>
#include <HepMC3/Data/GenParticleData.h>
#include <HepMC3/WriterAscii.h>

#include "src/Hepevt.hh"

using std::abs;
using std::cos;
using std::pow;
using std::sin;
using std::sqrt;

//---------------------------------------------------------------------------//
/*!
 * Read a CMS Pythia HEPEVT ASCII file and produce a HepMC3 ASCII output.
 */
void from_pythia(const char* pythia_input, const char* hepmc3_output)
{
    // Load pythia_input and create HepMC3 writer
    std::ifstream pythia_file(pythia_input);
    auto hepmc3_writer = std::make_shared<HepMC3::WriterAscii>(hepmc3_output);

    std::string  line;
    unsigned int event_number = 0;

    // Loop over pythia_input file lines until EOF is reached
    while (std::getline(pythia_file, line))
    {
        std::istringstream str_stream_header(line);
        event_number++;

        // Store header information
        hepmc3gen::HepevtHeader header;
        str_stream_header >> header.number_of_particles;
        header.event_number = event_number;

        // Create GenEvent that will be written in the hepmc3 output file
        HepMC3::GenEvent gen_event(HepMC3::Units::MEV, HepMC3::Units::CM);
        gen_event.set_event_number(header.event_number);

        // Loop over all particles in this event
        for (int i = 0; i < header.number_of_particles; i++)
        {
            std::getline(pythia_file, line);
            std::istringstream str_stream_particle(line);

            // Store particle information from the ascii file
            hepmc3gen::HepevtParticle particle;
            str_stream_particle >> particle.status >> particle.pdg
                >> particle.daughter_1 >> particle.daughter_2 >> particle.p_x
                >> particle.p_y >> particle.p_z >> particle.mass;

            if (particle.pdg != 22)
            {
                // Skip any particle that is not a photon
                continue;
            }

            // Calculate energy
            const double p_squared = particle.p_x * particle.p_x
                                     + particle.p_y * particle.p_y
                                     + particle.p_z * particle.p_z;
            const double m_squared = particle.mass * particle.mass;
            particle.energy        = std::sqrt(p_squared + m_squared);

            // Create and store GenParticle
            HepMC3::GenParticleData gen_particle_data;
            gen_particle_data.status      = particle.status;
            gen_particle_data.pid         = particle.pdg;
            gen_particle_data.is_mass_set = true;
            gen_particle_data.mass        = particle.mass;
            gen_particle_data.momentum
                = {particle.p_x, particle.p_y, particle.p_z, particle.energy};

            gen_event.add_particle(
                std::make_shared<HepMC3::GenParticle>(gen_particle_data));
        }
        hepmc3_writer->write_event(gen_event);
    }

    hepmc3_writer->close();
}

//---------------------------------------------------------------------------//
/*!
 * Create an isotropic distribution of photons from a point source.
 */
void create_isotropic(const char*        hepmc3_output,
                      const unsigned int num_events,
                      const unsigned int num_part_per_event,
                      const int          pdg_id,
                      const double       particle_energy)
{
    auto hepmc3_writer = std::make_shared<HepMC3::WriterAscii>(hepmc3_output);

    static constexpr double                pi = 3.141592653589793238;
    std::mt19937                           generator(12345);
    std::uniform_real_distribution<double> uniform_rng(0, 1);
    double                                 random_dir[3];

    for (int event = 0; event < num_events; event++)
    {
        HepMC3::GenEvent gen_event(HepMC3::Units::MEV, HepMC3::Units::CM);
        gen_event.set_event_number(event);

        for (int part_idx = 0; part_idx < num_part_per_event; part_idx++)
        {
            // Select a random uniform direction over a sphere
            const double theta = 2 * pi * uniform_rng(generator);
            const double phi   = acos(2 * uniform_rng(generator) - 1.0);
            random_dir[0]      = cos(theta) * sin(phi);
            random_dir[1]      = sin(theta) * sin(phi);
            random_dir[2]      = cos(phi);

            HepMC3::GenParticleData gen_particle_data;
            gen_particle_data.status      = 1; // Must be tracked
            gen_particle_data.pid         = pdg_id;
            gen_particle_data.momentum    = {particle_energy * random_dir[0],
                                          particle_energy * random_dir[1],
                                          particle_energy * random_dir[2],
                                          particle_energy};
            gen_particle_data.is_mass_set = true;

            switch (pdg_id)
            {
                case -11:
                    gen_particle_data.mass = 0.5109989461; //!< [MeV]
                    break;
                case 11:
                    gen_particle_data.mass = 0.5109989461; //!< [MeV]
                    break;
                case 22:
                    gen_particle_data.mass = 0;
                    break;
            }

            gen_event.add_particle(
                std::make_shared<HepMC3::GenParticle>(gen_particle_data));
        }
        hepmc3_writer->write_event(gen_event);
    }

    hepmc3_writer->close();
}

//---------------------------------------------------------------------------//
/*!
 * Produce HepMC3 input files for Celeritas demo-loop app.
 *
 * Possible HepMC3 files:
 * - Spherically isotropic source with fixed energy.
 * - Photons from original Pythia8 CMS HEPEVT files.
 *
 * Usages:
 * $ ./hepmc3-gen [isotropic.hepmc3] [num_events] [num_part_per_evt] [pdg_id]
 * [MeV_energy]
 * ./hepmc3-gen [cms_pythia_hepevt.data] [cms_pythia.hepmc3]
 *
 * Currently available PDGs are -11 (e+), 11 (e-), and 22 (gamma).
 */
int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        // Pythia input; Create hepmc3 output from it
        const char* pythia_input_file  = argv[1];
        const char* hepmc3_output_file = argv[2];

        from_pythia(pythia_input_file, hepmc3_output_file);
    }

    else if (argc == 6)
    {
        // Create isotropic distribution
        const char*        hepmc3_output_file = argv[1];
        const unsigned int num_events         = std::stoul(argv[2]);
        const unsigned int num_part_per_event = std::stoul(argv[3]);
        const int          pdg                = std::stoi(argv[4]);
        const double       energy             = std::stod(argv[5]);

        if (pdg != 22 && abs(pdg) != 11)
        {
            std::cout << "Currently available PDGs are -11 (e+), 11 (e-), and "
                         "22 (gamma)"
                      << std::endl;

            return EXIT_FAILURE;
        }

        create_isotropic(
            hepmc3_output_file, num_events, num_part_per_event, pdg, energy);
    }

    else
    {
        // Print help message
        std::cout << "Usage:" << std::endl;
        std::cout << argv[0]
                  << " [isotropic_out.hepmc3] [num_events] "
                     "[num_particles_per_event] [pdg_id] [particle_energy_MeV]"
                  << std::endl;
        std::cout << argv[0] << " [pythia_hepevt.data] [pythia_out.hepmc3]"
                  << std::endl;
        std::cout << "Currently available PDGs are -11 (e+), 11 (e-), and "
                     "22 (gamma)"
                  << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
