//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file src/Celeritas.cc
//---------------------------------------------------------------------------//
#include "Celeritas.hh"

#include <G4Threading.hh>
#include <accel/AlongStepFactory.hh>
#include <celeritas/em/UrbanMscParams.hh>
#include <celeritas/field/UniformFieldData.hh>
#include <celeritas/global/alongstep/AlongStepGeneralLinearAction.hh>
#include <celeritas/global/alongstep/AlongStepUniformMscAction.hh>
#include <celeritas/io/ImportData.hh>
#include <celeritas_version.h>

#include "JsonReader.hh"

using namespace celeritas;

namespace
{
//---------------------------------------------------------------------------//
std::shared_ptr<CoreStepActionInterface const>
make_nofield_along_step(AlongStepFactoryInput const& input)
{
    CELER_LOG(debug) << "Creating along-step action with linear propagation";
    std::shared_ptr<AlongStepGeneralLinearAction> asgla;

#if CELERITAS_VERSION == 0x000200
    asgla = celeritas::AlongStepGeneralLinearAction::from_params(
        input.action_id,
        *input.material,
        *input.particle,
        *input.physics,
        input.imported->em_params.energy_loss_fluct);
#else
    // Celeritas version is > 0.2.0, since cmake requires at least 0.2.0
    asgla = celeritas::AlongStepGeneralLinearAction::from_params(
        input.action_id,
        *input.material,
        *input.particle,
        celeritas::UrbanMscParams::from_import(
            *input.particle, *input.material, *input.imported),
        input.imported->em_params.energy_loss_fluct);
#endif

    return asgla;
}
//---------------------------------------------------------------------------//
}  // namespace

//---------------------------------------------------------------------------//
/*!
 * Globally shared setup options.
 */
SetupOptions& CelerSetupOptions()
{
    static SetupOptions options = [] {
        // Construct setup options the first time CelerSetupOptions is invoked
        SetupOptions so;

        // Set along-step factory
        so.make_along_step = UniformAlongStepFactory();

        so.max_num_tracks = 1024;
        so.max_num_events = 10000;
        so.initializer_capacity = 1024 * 128;
        so.secondary_stack_factor = 3.0;
        so.ignore_processes = {};

        // Use Celeritas "hit processor" to call back to Geant4 SDs.
        so.sd.enabled = true;

        // Only call back for nonzero energy depositions: this is currently a
        // global option for all detectors, so if any SDs extract data from
        // tracks with no local energy deposition over the step, it must be set
        // to false.
        so.sd.ignore_zero_deposition = true;

        // Using the pre-step point, reconstruct the G4 touchable handle.
        so.sd.locate_touchable = true;

        // Save diagnostic information
        so.output_file = "g4-validation-app.json";

        auto const& json = JsonReader::instance()->json();
        json.at("geometry").get_to(so.geometry_file);

        // Post-step data is used
        // so.sd.pre = {};
        so.sd.pre.position = true;
        so.sd.pre.global_time = true;
        return so;
    }();
    return options;
}

//---------------------------------------------------------------------------//
/*!
 * Celeritas problem data.
 */
SharedParams& CelerSharedParams()
{
    static SharedParams sp;
    return sp;
}

//---------------------------------------------------------------------------//
/*!
 * Thread-local (when supported) transporter.
 */
LocalTransporter& CelerLocalTransporter()
{
    static G4ThreadLocal LocalTransporter lt;
    return lt;
}
