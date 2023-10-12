//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Stopwatch.hh
//! \brief Simple CPU and wall timer.
//---------------------------------------------------------------------------//
#pragma once

#include <chrono>
#include <sys/time.h>

#include "RootData.hh"

//---------------------------------------------------------------------------//
/*!
 * Simple timer to calculate wall and CPU times during execution.
 * \code
 * Stopwatch stopwatch;
 * stopwatch.start();
 * // Do stuff
 * stopwatch.stop();
 * const double cpu_time = stopwatch.duration_cpu();
 * const double wall_time = stopwatch.duration_wall();
 * \endcode
 */
class Stopwatch
{
  public:
    // Construct empty
    Stopwatch();

    // Start stopwatch
    void start();

    // Stop stopwatch
    void stop();

    // Return duration [s]
    double duration_cpu();

    // Return duration [s]
    double duration_wall();

  private:
    using WallTime = std::chrono::high_resolution_clock::time_point;

    double   cpu_start_;
    double   cpu_stop_;
    WallTime wall_start_;
    WallTime wall_stop_;
};
