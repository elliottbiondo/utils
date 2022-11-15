#---------------------------------*-CMake-*----------------------------------#
# Copyright 2021-2022 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
project(g4-verify-gdml)

cmake_minimum_required(VERSION 3.9)
cmake_policy(VERSION 3.12...3.16)
set(CMAKE_CXX_STANDARD 17)

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
find_package(Geant4 REQUIRED gdml)

#----------------------------------------------------------------------------#
# Locate sources and headers of the project
#----------------------------------------------------------------------------#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.hh)

#----------------------------------------------------------------------------#
# Add executable
#----------------------------------------------------------------------------#
add_executable(verify-gdml verify-gdml.cc ${sources} ${headers})

target_include_directories(verify-gdml PRIVATE
 ${PROJECT_SOURCE_DIR}/src
 ${Geant4_INCLUDE_DIR}
)

target_link_libraries(verify-gdml PRIVATE
  ${Geant4_LIBRARIES}
)
