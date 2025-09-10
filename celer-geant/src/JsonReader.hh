//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/JsonReader.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>
#include <nlohmann/json.hpp>

//---------------------------------------------------------------------------//
/*!
 * Singleton \e nlohmann/json parser.
 *
 * Use \c JsonReader::Construct("input.json") to construct the singleton, and
 * \c JsonReader::Instance() to access it.
 *
 * \c Validate and \c ValidateHistogram are helper functions that call
 * \c CELER_VALIDATE on JSON input parameters.
 */
class JsonReader
{
  public:
    //! Construct by creating singleton from the json filename
    static void Construct(char const* json_filename);

    //! Instance singleton with json parser
    static nlohmann::json& Instance();

    //! Throw run-time error if key is not present
    static void Validate(nlohmann::json const& j, std::string name);

    //! Throw run-time error if JSON histogram keys are not present
    static void
    ValidateHistogram(nlohmann::json const& j, std::string hist_name);

  private:
    // JSON parser
    nlohmann::json json_;

    // Construct with filename
    JsonReader(char const* json_filename);
};
