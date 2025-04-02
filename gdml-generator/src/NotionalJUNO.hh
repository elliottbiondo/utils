//----------------------------------*-C++-*----------------------------------//
// Copyright 2024-2025 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file NotionalJUNO.hh
//! \brief Create the pmt geometry.
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <G4Material.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VUserDetectorConstruction.hh>

//---------------------------------------------------------------------------//
/*!
 * Construct a notional JUNO geometry.
 *
 * This geometry consists of an arbitrary number of small, spherical pmts
 * arranged with (nearly*]) equal spacing in a spherical shell configuration,
 * mimicking the JUNO neutrino pmt. This is achieved using the Fibbonaci
 * Sphere algorithm. Here, the radius of the spherical shell is refered to as
 * the "device_radius".
 */
class NotionalJUNO : public G4VUserDetectorConstruction
{
  public:
    //>> TYPES
    using Real3 = std::array<double, 3>;
    using VecReal3 = std::vector<Real3>;

  public:
    // Construct
    NotionalJUNO(double device_radius, double pmt_radius, int num_pmts);

    // Construct geometry
    G4VPhysicalVolume* Construct() final;

    // Set up sensitive pmts and magnetic field
    void ConstructSDandField() final;

  private:
    double device_radius_;
    double pmt_radius_;
    int num_pmts_;
    VecReal3 points_;
};
