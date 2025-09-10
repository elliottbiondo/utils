//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celer-geant/src/JsonReader.cc
//---------------------------------------------------------------------------//
#include "JsonReader.hh"

#include <fstream>
#include <corecel/Assert.hh>

//---------------------------------------------------------------------------//
//! Singleton declaration.
static JsonReader* json_reader_singleton{nullptr};

//---------------------------------------------------------------------------//
/*!
 * Construct singleton with JSON filename.
 */
void JsonReader::Construct(char const* json_filename)
{
    CELER_VALIDATE(!json_reader_singleton, << "JsonReader already constructed");
    json_reader_singleton = new JsonReader(json_filename);
}

//---------------------------------------------------------------------------//
/*!
 * Get JSON parser.
 *
 * \note \c JsonReader::construct(filename) must be called to construct it.
 */
nlohmann::json& JsonReader::Instance()
{
    CELER_VALIDATE(json_reader_singleton,
                   << "JsonReader not constructed. Initialize it by calling "
                      "JsonReader::construct(filename).");
    return json_reader_singleton->json_;
}

//---------------------------------------------------------------------------//
/*!
 * Throw run-time error if JSON key is not present.
 */
void JsonReader::Validate(nlohmann::json const& j, std::string name)
{
    CELER_VALIDATE(j.contains(name),
                   << "Missing \"" << name << "\" in JSON input.");
}

//---------------------------------------------------------------------------//
/*!
 * Throw run-time error if JSON histogram keys are not present.
 */
void JsonReader::ValidateHistogram(nlohmann::json const& j,
                                   std::string hist_name)
{
    JsonReader::Validate(j, hist_name);
    auto const& jh = j.at(hist_name);

#define JR_HIST_VALIDATE(MEMBER)                                        \
    CELER_VALIDATE(jh.contains(#MEMBER),                                \
                   << "Histogram \"" << hist_name << "\" is missing \"" \
                   << #MEMBER << "\" in JSON input.");

    JR_HIST_VALIDATE(num_bins)
    JR_HIST_VALIDATE(min)
    JR_HIST_VALIDATE(max)

#undef JR_HIST_VALIDATE
}

//---------------------------------------------------------------------------//
// PRIVATE
//---------------------------------------------------------------------------//
/*!
 * Construct from JSON filename.
 */
JsonReader::JsonReader(char const* json_filename)
{
    json_ = nlohmann::json::parse(std::ifstream(json_filename));
    CELER_VALIDATE(!json_.is_null(),
                   << "'" << json_filename << "' is not a valid input");
}
