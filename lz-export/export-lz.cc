//------------------------------- -*- C++ -*- -------------------------------//
// Copyright Celeritas contributors: see top-level COPYRIGHT file for details
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file export-lz.cc
//---------------------------------------------------------------------------//
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

// NOTE: obtain from baccarat, e.g.:
// https://gitlab.com/luxzeplin/CeleritasDev/baccarat/-/blob/master/LZ/include/LZDetectorPMTBankCoordinates.hh
#include "LZDetectorPMTBankCoordinates.hh"

using nlohmann::json;
using std::size;

int main()
{
    // Helper function to convert 2D array to vector of vector of doubles
    auto convertArrayToVectors = [](double const(*array)[2], size_t size) {
        std::vector<std::vector<double>> result;
        result.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            result.push_back({array[i][0], array[i][1]});
        }
        return result;
    };

    // Create JSON object
    json j;
    j["bottom"] = convertArrayToVectors(BottomPMTRodArrayXY,
                                        std::size(BottomPMTRodArrayXY));
    j["top"] = convertArrayToVectors(TopPMTArrayXY, std::size(TopPMTArrayXY));

    // Print to console
    std::cout << j.dump(1) << std::endl;

    return 0;
}
