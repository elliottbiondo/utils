//----------------------------------*-C++-*----------------------------------//
// Copyright 2024-2026 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file NotionalDUNE.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4VUserDetectorConstruction.hh>

class G4VPhysicalVolume;

//---------------------------------------------------------------------------//
/*!
 * Construct a notional DUNE geometry.
 *
 * A central 1 m x 1 m x 1 m cube of liquid argon, centered at the origin, is
 * filled with a regular N x N x N grid of equally spaced spheres as backgroud
 * volume stand-ins for the anode. The liquid-argon volume is then nested inside
 * M successively larger concentric vacuum boxes, each adding
 * level_thickness of wall on every side.
 */
class NotionalDUNE final : public G4VUserDetectorConstruction
{
  public:
    NotionalDUNE(int num_spheres_per_axis, int num_shells);

    G4VPhysicalVolume* Construct() final;
    void ConstructSDandField() final;

  private:
    //! Edge length of the central liquid-argon box [cm]
    static constexpr double box_size_ = 100;
    //! Radius of each "anode" sphere [cm]
    static constexpr double sphere_radius_ = 0.25;
    //! Wall thickness added per concentric vacuum shell [cm]
    static constexpr double shell_thickness_ = 1;
    //! Spheres per axis (N), N^3 spheres total
    int num_spheres_per_axis_;
    //! Number of concentric outer vacuum boxes (M)
    int num_shells_;
};
