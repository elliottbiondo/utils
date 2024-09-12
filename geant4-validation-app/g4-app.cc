//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file g4-app.cc
//! \brief Geant4 validation app.
//---------------------------------------------------------------------------//
#include <string>
#include <G4GDMLParser.hh>
#include <G4TransportationManager.hh>
#include <celeritas/ext/GeantImporter.hh>
#include <celeritas/ext/RootExporter.hh>
#include <corecel/sys/ScopedMpiInit.hh>

#include "src/G4appMacros.hh"
#include "src/Geant4Run.hh"
#include "src/HepMC3Reader.hh"
#include "src/RootIO.hh"
#include "src/Stopwatch.hh"

using std::cout;
using std::endl;

//---------------------------------------------------------------------------//
/*!
 * Export a Celeritas demo-loop ROOT input file with the same physics options
 * used by the Geant4 validation app.
 */
void export_celeritas_root_input(G4VPhysicalVolume* world_volume)
{
#if USE_ROOT
    celeritas::ScopedMpiInit scoped_mpi;
    celeritas::GeantImporter import_data(world_volume);
    celeritas::RootExporter export_root("celeritas-demo-loop-input.root");
    export_root(import_data());
#else
    cout << "ERROR: Cannot generate the Celeritas ROOT output file without "
            "ROOT. Recompile with USE_ROOT=ON."
         << endl;
#endif
}

//---------------------------------------------------------------------------//
/*!
 * Geant4 validation app. Input options are selected via a json file.
 *
 * Usage:
 * $ ./g4app input.json
 * $ ./g4app input.json output.root
 *
 * See input_example.json and README.md.
 */
int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3)
    {
        // Print help message
        cout << "Usage:" << endl;
        cout << argv[0] << " input_options.json" << endl;
        cout << argv[0] << " input_options.json output.root" << endl;
        return EXIT_FAILURE;
    }

    // Open fstream for json input file
    std::ifstream json_input_stream(argv[1]);
    if (!json_input_stream.is_open())
    {
        // File not found
        cout << "File " << argv[1] << " not found." << endl;
        return EXIT_FAILURE;
    }

    // Define if ROOT output should be stored
    bool const is_root_output_enabled = (argc == 3);
    if (is_root_output_enabled && !USE_ROOT)
    {
        // Cannot write ROOT file without ROOT dependency
        cout << "ERROR: Cannot generate " << argv[2]
             << " output file without ROOT. Recompile with USE_ROOT=ON."
             << endl;
        return EXIT_FAILURE;
    }

    // Create stopwatch for total wall/cpu times
    Stopwatch stopwatch_total;
    stopwatch_total.start();

    // >>> INITIALIZE INPUT READERS AND I/O

    // Construct json reader
    JsonReader::construct(json_input_stream);
    auto const json = JsonReader::instance()->json();

    std::string const hepmc3_input
        = json.at("simulation").at("hepmc3").get<std::string>();
    if (!hepmc3_input.empty())
    {
        HepMC3Reader::construct();
    }

    if (is_root_output_enabled)
    {
        RootIO::construct(argv[2]);
    }

    // >>> EXECUTE GEANT4 RUN

    // Initialize Geant4 and clock its simulation wall/cpu times
    Geant4Run geant4_run;
    Stopwatch stopwatch_beamon;
    stopwatch_beamon.start();
    geant4_run.beam_on();
    stopwatch_beamon.stop();

    // Stop total simulation clock
    stopwatch_total.stop();

    // >>> PERFORMANCE METRICS AND ROOT DATA

    rootdata::ExecutionTime exec_time;
    exec_time.wall_total = stopwatch_total.duration_wall();
    exec_time.cpu_total = stopwatch_total.duration_cpu();
    exec_time.wall_sim_run = stopwatch_beamon.duration_wall();
    exec_time.cpu_sim_run = stopwatch_beamon.duration_cpu();
    exec_time.print();

    if (is_root_output_enabled && USE_ROOT)
    {
        auto root_io = RootIO::instance();
        root_io->store_input();
        root_io->store_performance_metrics(exec_time);

        if (!root_io->is_performance_run())
        {
            // Store SD data
            root_io->store_sd_map();
        }
        root_io->write_tfile();
    }

    // >>> EXPORT CELERITAS' ROOT INPUT FILE

    if (json.at("export_celeritas_root").get<bool>())
    {
        export_celeritas_root_input(geant4_run.world_volume());
    }

    return EXIT_SUCCESS;
}
