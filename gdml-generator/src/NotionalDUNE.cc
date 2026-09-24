//----------------------------------*-C++-*----------------------------------//
// Copyright 2024-2025 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file NotionalDUNE.cc
//---------------------------------------------------------------------------//
#include "NotionalDUNE.hh"

#include <G4Box.hh>
#include <G4LogicalVolume.hh>
#include <G4NistManager.hh>
#include <G4Orb.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4ThreeVector.hh>

#include "core/SensitiveDetector.hh"

//---------------------------------------------------------------------------//
/*!
 * Constructor.
 */
NotionalDUNE::NotionalDUNE(int num_spheres_per_axis, int num_shells)
    : num_spheres_per_axis_(num_spheres_per_axis)
    , num_shells_(num_shells)
{
}

//---------------------------------------------------------------------------//
/*!
 * Build a notional DUNE geometry.
 */
G4VPhysicalVolume* NotionalDUNE::Construct()
{
    // Materials
    auto nist = G4NistManager::Instance();
    // Bulk medium of the TPC: liquid argon
    auto lar_mat = nist->FindOrBuildMaterial("G4_lAr");
    // Anode material. Copper is used as a representative stand-in
    auto anode_mat = nist->FindOrBuildMaterial("G4_Cu");
    // Everything outside the active volume (world + concentric shells): vacuum
    auto world_mat = nist->FindOrBuildMaterial("G4_Galactic");
    world_mat->SetName("vacuum");

    // Concentric vacuum boxes, built from the outermost shell inward so
    // that each shell is placed inside the next-larger one. The outermost box
    // is the world, so num_shells_ enclosing boxes give num_shells_ + 1
    // universe levels in ORANGE (the last being the liquid argon).
    G4VPhysicalVolume* world_pv = nullptr;
    G4LogicalVolume* mother_lv = nullptr;
    for (int shell = num_shells_; shell >= 1; --shell)
    {
        double const edge = box_size_ + 2.0 * shell * shell_thickness_;
        G4Box* shell_box = new G4Box(
            "shell_box", 0.5 * edge * cm, 0.5 * edge * cm, 0.5 * edge * cm);
        auto const shell_lv
            = new G4LogicalVolume(shell_box, world_mat, "shell_lv");
        auto const shell_pv = new G4PVPlacement(nullptr,
                                                G4ThreeVector(),
                                                shell_lv,
                                                "shell_pv",
                                                mother_lv,
                                                false,
                                                shell,
                                                false);
        if (!world_pv)
        {
            world_pv = shell_pv;
        }
        mother_lv = shell_lv;
    }

    // Central liquid-argon box that holds the grid of anode spheres, placed
    // in the innermost shell, or as the world itself when num_shells_ == 0 (a
    // single universe with an implicit LAr background).
    G4Box* inner_box = new G4Box("inner_box",
                                 0.5 * box_size_ * cm,
                                 0.5 * box_size_ * cm,
                                 0.5 * box_size_ * cm);
    auto const inner_lv = new G4LogicalVolume(inner_box, lar_mat, "inner_lv");
    auto const inner_pv = new G4PVPlacement(
        nullptr, G4ThreeVector(), inner_lv, "inner_pv", mother_lv, false, 0, false);
    if (!world_pv)
    {
        world_pv = inner_pv;
    }

    // Grid of N^3 "anode" spheres, equally spaced and symmetric about the 
    // origin.
    double const pitch = box_size_ / num_spheres_per_axis_;
    double const offset = -0.5 * box_size_ + 0.5 * pitch;

    G4Orb* sphere = new G4Orb("sphere", sphere_radius_ * cm);
    auto const sphere_lv = new G4LogicalVolume(sphere, anode_mat, "sphere_lv");

    int copy_no = 0;
    for (int i = 0; i < num_spheres_per_axis_; ++i)
    {
        for (int j = 0; j < num_spheres_per_axis_; ++j)
        {
            for (int k = 0; k < num_spheres_per_axis_; ++k)
            {
                G4ThreeVector pos((offset + i * pitch) * cm,
                                  (offset + j * pitch) * cm,
                                  (offset + k * pitch) * cm);
                new G4PVPlacement(nullptr,
                                  pos,
                                  sphere_lv,
                                  "sphere_pv",
                                  inner_lv,
                                  false,
                                  copy_no++,
                                  false);
            }
        }
    }

    return world_pv;
}

//---------------------------------------------------------------------------//
/*!
 * Set the bulk liquid-argon as the sensitive detector.
 */
void NotionalDUNE::ConstructSDandField()
{
    auto lar_sd = new SensitiveDetector("lar_sd");
    G4SDManager::GetSDMpointer()->AddNewDetector(lar_sd);
    G4VUserDetectorConstruction::SetSensitiveDetector("inner_lv", lar_sd);
}
