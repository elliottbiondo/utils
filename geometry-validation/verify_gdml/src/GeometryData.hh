//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Data.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>

//---------------------------------------------------------------------------//
/*!
 * Store volume information.
 */
struct Volume
{
    int         volume_id;
    int         material_id;
    std::string material_name;
    std::string physical_volume_name;
    std::string logical_volume_name;
    int         copy_num;
    int         num_replicas;
};
