//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file RootInterfaceLinkDef.hh
//! \brief Classes added to the ROOT dictionary.
//---------------------------------------------------------------------------//
#ifdef __CINT__

// clang-format off
#pragma link C++ class rootdata::Array3+;
#pragma link C++ class rootdata::SensDetScoreData+;
#pragma link C++ class rootdata::SensDetGdml+;
#pragma link C++ class rootdata::Step+;
#pragma link C++ class rootdata::Track+;
#pragma link C++ class rootdata::Event+;
#pragma link C++ class rootdata::ExecutionTime+;
#pragma link C++ class rootdata::DataLimits+;
// clang-format on

#endif
