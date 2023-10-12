//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RootUniquePtr.hh
//! \brief Helpers to prevent ROOT from propagating to downstream code.
//---------------------------------------------------------------------------//
#pragma once

#include <memory>

#include "G4appMacros.hh"

// Forward-declare ROOT; Expand as needed
class TFile;
class TTree;

//---------------------------------------------------------------------------//
/*!
 * Unique_ptr deleter.
 */
template<class T>
struct DeleteRoot
{
    void operator()(T*) const;
};

template<class T>
using RootUP = std::unique_ptr<T, DeleteRoot<T>>;

//---------------------------------------------------------------------------//
#if !USE_ROOT
template<class T>
void DeleteRoot<T>::operator()(T*) const
{
    __builtin_unreachable();
}
#endif
