//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/RootDataStore.cc
//---------------------------------------------------------------------------//
#include "RootDataStore.hh"

#include <corecel/Assert.hh>

//---------------------------------------------------------------------------//
/*!
 * Add physical volume ID and copy number to the map and initialize histograms
 * associated to it.
 */
void RootDataStore::InsertSensDet(PhysVolId pid,
                                  CopyNumber cid,
                                  std::string name)
{
    sensdet_map_.insert({{pid, cid}, SensDetData::Initialize(name)});
}

//---------------------------------------------------------------------------//
/*!
 * Return data for a given sensitive detector.
 */
SensDetData& RootDataStore::Find(PhysVolId pv_id, CopyNumber copy_num)
{
    auto iter = sensdet_map_.find(SensDetId{pv_id, copy_num});
    CELER_ASSERT(iter != sensdet_map_.end());
    return iter->second;
}
