//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Stopwatch.cc
//---------------------------------------------------------------------------//
#include "Stopwatch.hh"

//---------------------------------------------------------------------------//
/*!
 * Construct empty.
 */
Stopwatch::Stopwatch() {}

//---------------------------------------------------------------------------//
/*!
 * Start stopwatch.
 */
void Stopwatch::start()
{
    cpu_start_ = std::clock();  // [ms]
    wall_start_ = std::chrono::high_resolution_clock::now();
}

//---------------------------------------------------------------------------//
/*!
 * Stop stopwatch.
 */
void Stopwatch::stop()
{
    cpu_stop_ = std::clock();  // [ms]
    wall_stop_ = std::chrono::high_resolution_clock::now();
}

//---------------------------------------------------------------------------//
/*!
 * Return CPU duration in [s].
 */
double Stopwatch::duration_cpu()
{
    return (cpu_stop_ - cpu_start_) / CLOCKS_PER_SEC;
}

//---------------------------------------------------------------------------//
/*!
 * Return wall time duration in [s].
 */
double Stopwatch::duration_wall()
{
    return static_cast<double>(
               std::chrono::duration_cast<std::chrono::microseconds>(
                   wall_stop_ - wall_start_)
                   .count())
           / CLOCKS_PER_SEC;
}
