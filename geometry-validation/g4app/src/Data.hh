//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
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
    unsigned int material_id;
    std::string  material_name;
    std::string  name;
    std::string  solid_name;
};
