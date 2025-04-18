//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file export-lz.cc
//---------------------------------------------------------------------------//
#include <iostream>
#include <span>
#include <vector>
#include <nlohmann/json.hpp>

// NOTE: obtain from baccarat, e.g.:
// https://gitlab.com/luxzeplin/CeleritasDev/baccarat/-/blob/master/LZ/include/LZDetectorPMTBankCoordinates.hh
#include "LZDetectorPMTBankCoordinates.hh"

// From produceS2HitMaps:
constexpr double topArrayZ{1539.5};  // Z position of PMT arrays
constexpr double bottomArrayZ{-148};

using nlohmann::json;

// Helper function to convert 2D array to vector of vector of doubles
auto convertArrayToVectors(std::span<double const[2]> array)
{
    std::vector<std::vector<double>> result;
    result.reserve(array.size());
    for (size_t i = 0; i < array.size(); i++)
    {
        result.push_back({array[i][0], array[i][1]});
    }
    return result;
}

int main()
{
    // Create JSON object
    json j;
    j["bottom"]
        = {{"z", bottomArrayZ},
           {"xy", convertArrayToVectors(std::span{BottomPMTRodArrayXY})}};
    j["top"] = {{"z", topArrayZ},
                {"xy", convertArrayToVectors(std::span{TopPMTArrayXY})}};

    // Print to console
    std::cout << j.dump(0) << std::endl;

    return 0;
}
