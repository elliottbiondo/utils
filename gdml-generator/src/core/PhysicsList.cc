//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file PhysicsList.cc
//---------------------------------------------------------------------------//
#include "PhysicsList.hh"

#include <G4Electron.hh>
#include <G4Gamma.hh>
#include <G4Positron.hh>
#include <G4ProductionCuts.hh>
#include <G4ProductionCutsTable.hh>
#include <G4Proton.hh>
#include <G4Region.hh>
#include <G4RegionStore.hh>
#include <G4TransportationManager.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct with range cuts value [mm].
 */
PhysicsList::PhysicsList(double range_cuts)
    : G4VUserPhysicsList(), range_cuts_(range_cuts)
{
}

//---------------------------------------------------------------------------//
/*!
 * Build list of available particles.
 *
 * The minimal EM set can be built by using
 * \c G4EmBuilder::ConstructMinimalEmSet();
 * and includes gamma, e+, e-, mu+, mu-, pi+, pi-, K+, K-, p, pbar, deuteron,
 * triton, He3, alpha, and generic ion, along with Geant4's pseudo-particles
 * geantino and charged geantino.
 *
 * Currently only instantiating e+, e-, gamma, and proton. The latter is needed
 * for msc and production cut tables.
 */
void PhysicsList::ConstructParticle()
{
    G4Gamma::GammaDefinition();
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
    G4Proton::ProtonDefinition();
}

//---------------------------------------------------------------------------//
/*!
 * Construct mandatory processes.
 */
void PhysicsList::ConstructProcess()
{
    // Add mandatory transportation
    G4VUserPhysicsList::AddTransportation();
}

//---------------------------------------------------------------------------//
/*!
 * Set different production cut thresholds for different test cases.
 *
 * \note
 * AdePT's TestEm3 uses 0.7 mm.
 */
void PhysicsList::SetCuts()
{
    // Get the existing default region instead of creating a new one
    auto region_store = G4RegionStore::GetInstance();
    auto region = region_store->GetRegion(PhysicsList::default_region_name());
    if (!region)
    {
        // Fallback: if the default region doesn't exist, create it
        region = new G4Region(PhysicsList::default_region_name());
        auto world = G4TransportationManager::GetTransportationManager()
                         ->GetNavigatorForTracking()
                         ->GetWorldVolume()
                         ->GetLogicalVolume();
        assert(world);
        region->AddRootLogicalVolume(world);
        region->UsedInMassGeometry(true);
    }

    // Initialize production cuts
    auto prod_cuts = new G4ProductionCuts();
    prod_cuts->SetProductionCut(range_cuts_);  // [mm]
    region->SetProductionCuts(prod_cuts);

    // Update production cuts table with new values
    auto prod_cuts_table = G4ProductionCutsTable::GetProductionCutsTable();
    prod_cuts_table->UpdateCoupleTable(
        G4TransportationManager::GetTransportationManager()
            ->GetNavigatorForTracking()
            ->GetWorldVolume());
}
