//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file src/Celeritas.hh
//---------------------------------------------------------------------------//
#pragma once

#include <accel/LocalTransporter.hh>
#include <accel/SetupOptions.hh>
#include <accel/SharedParams.hh>

// Global shared setup options
celeritas::SetupOptions& CelerSetupOptions();
// Shared data and GPU setup
celeritas::SharedParams& CelerSharedParams();
// Thread-local transporter
celeritas::LocalTransporter& CelerLocalTransporter();
