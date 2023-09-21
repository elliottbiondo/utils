//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file gdml-subset.cc
//---------------------------------------------------------------------------//
#include "corecel/Assert.hh"
#include "corecel/cont/Range.hh"
#include "corecel/io/Logger.hh"
#include "corecel/io/Join.hh"
#include "celeritas/ext/GeantGeoUtils.hh"
#include "celeritas/ext/ScopedGeantExceptionHandler.hh"
#include "celeritas/ext/ScopedGeantLogger.hh"

#include <cstdlib>
#include <string>
#include <vector>
#include <G4GDMLParser.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Version.hh>

using namespace celeritas;

//---------------------------------------------------------------------------//
void print_usage(char const* exec_name)
{
    std::cerr
        << "usage: " << exec_name << " {input}.gdml {physvol-name} {depth} {output}.gdml\n";
}

void delete_daughters_after(G4LogicalVolume* lv, int depth)
{
    if (depth == 0)
    {
        // Delete daughters
        lv->ClearDaughters();
        return;
    }
    --depth;

    for (auto const i : celeritas::range(lv->GetNoDaughters()))
    {
        delete_daughters_after(lv->GetDaughter(i)->GetLogicalVolume(), depth);
    }
}

//---------------------------------------------------------------------------//
void run(std::string const& inp_filename, std::string const& vol_name, int depth, std::string const& out_filename)
{
    // Read geometry *without* stripping pointers
    celeritas::load_geant_geometry(inp_filename);

    // Find volume
    auto& pvs = *G4PhysicalVolumeStore::GetInstance();
    auto new_world = std::find_if(
        pvs.begin(), pvs.end(), [&vol_name](G4VPhysicalVolume* pv) {
            return pv && pv->GetName() == vol_name;
        });

    CELER_VALIDATE(
        new_world != pvs.end(),
        << "failed to find volume '" << vol_name << "': available names are "
        << celeritas::join(
               pvs.begin(), pvs.end(), ", ", [](G4VPhysicalVolume* pv) {
                   return pv ? pv->GetName() : "<NULL>";
               }));

    // Trim insides
    delete_daughters_after((*new_world)->GetLogicalVolume(), depth);

    // Write output
    G4GDMLParser parser;
    parser.SetEnergyCutsExport(false);
    parser.SetSDExport(false);
    parser.SetOverlapCheck(false);
#if G4VERSION_NUMBER >= 1070
    parser.SetOutputFileOverwrite(true);
#endif

    parser.Write(out_filename, *new_world, /* append_pointers = */ false);
}

int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.size() == 1 && (args.front() == "--help" || args.front() == "-h"))
    {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    if (args.size() != 4)
    {
        // Incorrect number of arguments: print help and exit
        print_usage(argv[0]);
        return 2;
    }

    try
    {
        ScopedGeantLogger scoped_log_;
        ScopedGeantExceptionHandler scoped_exceptions_;
        run(args[0], args[1], std::atoi(args[2].c_str()), args[3]);
    }
    catch (RuntimeError const& e)
    {
        CELER_LOG(critical) << "Runtime error: " << e.what();
        return EXIT_FAILURE;
    }
    catch (DebugError const& e)
    {
        CELER_LOG(critical) << "Assertion failure: " << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
