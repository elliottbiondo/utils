#------------------------------- -*- cmake -*- -------------------------------#
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.12)
project(lz-export VERSION 0.1.0 LANGUAGES CXX)

find_package(nlohmann_json REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the executable
add_executable(export-lz export-lz.cc)
target_link_libraries(export-lz PRIVATE nlohmann_json::nlohmann_json)

#-----------------------------------------------------------------------------#
