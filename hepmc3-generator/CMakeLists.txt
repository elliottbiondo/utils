#---------------------------------*-CMake-*----------------------------------#
# Copyright 2021-2024 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.10)
project(hepmc3-generator)

set(CMAKE_CXX_STANDARD 11)

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
find_package(HepMC3 REQUIRED)

#----------------------------------------------------------------------------#
# Add executable
#----------------------------------------------------------------------------#
add_executable(hepmc3-gen hepmc3-gen.cc
  src/Hepevt.hh
)

target_include_directories(hepmc3-gen PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${HEPMC3_INCLUDE_DIR}
)

target_link_libraries(hepmc3-gen PRIVATE
  ${HEPMC3_LIB}
)
