#---------------------------------*-CMake-*----------------------------------#
# Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.9)
project(gdml-generator)

cmake_policy(VERSION 3.12...3.16)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
find_package(Geant4 REQUIRED gdml)

#----------------------------------------------------------------------------#
# Locate sources and headers of the project
#----------------------------------------------------------------------------#
set(core_dir ${PROJECT_SOURCE_DIR}/src/core)
set(geo_dir ${PROJECT_SOURCE_DIR}/src)

set(core_src
  ${core_dir}/PhysicsList.cc
  ${core_dir}/SensitiveDetector.cc
)

set(geo_src
  ${geo_dir}/BoxDetector.cc
  ${geo_dir}/SimpleCMSDetector.cc
  ${geo_dir}/TestEm3Detector.cc
)

#----------------------------------------------------------------------------#
# Add executable
#----------------------------------------------------------------------------#
add_executable(gdml-gen gdml-gen.cc
  ${core_src}
  ${geo_src}
)

target_include_directories(gdml-gen PRIVATE
  $<BUILD_INTERFACE:
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Geant4_INCLUDE_DIR}>
)

target_link_libraries(gdml-gen PRIVATE ${Geant4_LIBRARIES})
