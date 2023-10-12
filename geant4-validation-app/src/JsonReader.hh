//----------------------------------*-C++-*----------------------------------//
// Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file JsonReader.hh
//! \brief nlohmann/json singleton.
//---------------------------------------------------------------------------//
#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

//---------------------------------------------------------------------------//
/*!
 * JSON singleton for allowing global access to the \e nlohmann/json parser.
 * Use \c JsonReader::construct("input.json") to construct the singleton, and
 * \c JsonReader::instance() to access it from any class.
 */
class JsonReader
{
  public:
    // Construct by creating singleton from the json filename
    static void construct(std::ifstream& json_filename);

    // Get singleton instance
    static JsonReader* instance();

    // Get parsed json for reading
    nlohmann::json& json();

  private:
    // Construct with filename
    JsonReader(std::ifstream& json_filename);

  private:
    nlohmann::json json_;
};
