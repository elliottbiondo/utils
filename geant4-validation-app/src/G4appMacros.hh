//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file G4appMacros.hh
//! \brief Compile macros.
//---------------------------------------------------------------------------//
#pragma once

#include <G4Version.hh>

//---------------------------------------------------------------------------//
/*!
 * Geant4 v10 vs v11.
 */
#if defined(G4VERSION_NUMBER) && G4VERSION_NUMBER < 1100
#    define G4_V10 1
#else
#    define G4_V10 0
#endif

//---------------------------------------------------------------------------//
/*!
 * ROOT option.
 */
#if defined(G4APP_USE_ROOT)
#    define USE_ROOT 1
#else
#    define USE_ROOT 0
#endif

//---------------------------------------------------------------------------//
/*!
 * Multithread option.
 */
#if defined(G4MULTITHREADED) && defined(G4APP_USE_MT)
#    define USE_MT 1
#else
#    define USE_MT 0
#endif

//---------------------------------------------------------------------------//
/*!
 * QT option.
 */
#if defined(G4APP_USE_QT)
#    define USE_QT 1
#else
#    define USE_QT 0
#endif
