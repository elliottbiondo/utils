//----------------------------------*-C++-*----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file DetectorConstruction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <memory>
#include <G4VUserDetectorConstruction.hh>
#include <G4VPhysicalVolume.hh>

//---------------------------------------------------------------------------//
/*!
 * Safely select different geometry options.
 */
enum class Geometry
{
    simple_cms,
    example_b1,
    four_steel_slabs,
    ams_ecal
};

//---------------------------------------------------------------------------//
/*!
 * Construct detector geometry either programmatically or via a gdml input.
 *
 * For comparison tests, the user must run the code using a programmatic
 * geometry *BEFORE* running the gdml parsing test.
 *
 * If the programmatic option is selected, the selected geometry is also
 * exported as gdml, in preparation for the gdml parsing test. Geant4
 * singletons do not allow for a correct execution if one tries to create a
 * programmatic geometry, export it as gdml, reset geometry data, then parse
 * the exported gdml file.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    // Construct
    DetectorConstruction(Geometry selection, bool from_gdml);

    // Default destructor
    ~DetectorConstruction() = default;

    // Get constructed world volume
    const G4VPhysicalVolume* get_world_volume() const;

    // Get hardcoded gdml filename
    std::string get_gdml_filename() { return gdml_filename_; }

    // Construct geometry for a Geant4 simulation run
    G4VPhysicalVolume* Construct() override;

  private:
    // Export GDML file representing the programmatic geometry
    void export_gdml();

    // Set physical volume pointer
    void set_phys_volume();

    // Set up gdml filename accordingly
    void set_gdml_filename();

    // Create programmatic geometry: single-material CMS mock up
    G4VPhysicalVolume* create_simple_cms_geometry();

    // Create programmatic geometry: Geant4/examples/basic/B1
    G4VPhysicalVolume* create_b1_geometry();

    // Create programmatic geometry: Celeritas' four-steel-slabs.gdml
    G4VPhysicalVolume* create_slabs_geometry();

    // Create programmatic geometry: Geant4/examples/advanced/amsEcal
    G4VPhysicalVolume* create_ams_ecal_geometry();

  private:
    std::unique_ptr<G4VPhysicalVolume> world_phys_vol_;
    std::string                        gdml_filename_;
    Geometry                           selected_geometry_;
};
