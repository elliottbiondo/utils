//----------------------------------*-C++-*----------------------------------//
// Copyright 2024-2025 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file NotionalJUNO.cc
//---------------------------------------------------------------------------//
#define _USE_MATH_DEFINES

#include "NotionalJUNO.hh"

#include <cmath>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4Orb.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Constructor
 */
NotionalJUNO::NotionalJUNO(double device_radius, double pmt_radius, int num_pmts)
    : device_radius_(device_radius)
    , pmt_radius_(pmt_radius)
    , num_pmts_(num_pmts)
{
    // Create a vector of pmt centers using the Fibonacci sphere algorithm
    points_.resize(num_pmts_);

    double golden_ratio = (1 + sqrt(5)) / 2;
    double k = 2 * M_PI / golden_ratio;

    for (int i = 0; i < num_pmts_; ++i)
    {
        double theta = k * i;
        double phi = acos(1 - 2 * static_cast<double>(i) / num_pmts_);

        points_[i]
            = NotionalJUNO::Real3({device_radius_ * cos(theta) * sin(phi),
                                   device_radius_ * sin(theta) * sin(phi),
                                   device_radius_ * cos(phi)});
    }
}

//---------------------------------------------------------------------------//
/*!
 * Build a notional JUNO geometry.
 */
G4VPhysicalVolume* NotionalJUNO::Construct()
{
    // Materials
    auto nist = G4NistManager::Instance();
    auto pmt_mat = nist->FindOrBuildMaterial("G4_Au");
    auto world_mat = nist->FindOrBuildMaterial("G4_Galactic");
    world_mat->SetName("vacuum");

    // World Cylinder
    G4Orb* world_sphere
        = new G4Orb("world_sphere", (device_radius_ + pmt_radius_) * cm);
    auto const world_lv = new G4LogicalVolume(world_sphere, world_mat, "world");
    auto const world_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), world_lv, "world_pv", nullptr, false, 0, false);

    // Detector
    G4Orb* pmt = new G4Orb("pmt", pmt_radius_ * cm);
    auto const pmt_lv = new G4LogicalVolume(pmt, pmt_mat, "pmt_lv");

    // Detector placements
    for (auto i = 0; i < points_.size(); ++i)
    {
        new G4PVPlacement(nullptr,
                          G4ThreeVector(points_[i][0] * cm,
                                        points_[i][1] * cm,
                                        points_[i][2] * cm),
                          pmt_lv,
                          "pmt_pv",
                          world_lv,
                          false,
                          i,
                          false);
    }

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set sensitive pmts.
 */
void NotionalJUNO::ConstructSDandField()
{
    auto pmt_sd = new SensitiveDetector("pmt_sd");
    G4SDManager::GetSDMpointer()->AddNewDetector(pmt_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("pmt_lv", pmt_sd);
}
