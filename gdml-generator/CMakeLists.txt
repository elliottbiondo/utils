#---------------------------------*-CMake-*----------------------------------#
# Copyright 2022-2024 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.15)
project(gdml-generator)

cmake_policy(VERSION 3.15...3.23)

if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  message(STATUS "Set default CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
find_package(Geant4 REQUIRED gdml)

#----------------------------------------------------------------------------#
# Add executable
#----------------------------------------------------------------------------#

add_executable(gdml-gen gdml-gen.cc
  src/core/PhysicsList.cc
  src/core/SensitiveDetector.cc
  src/BoxDetector.cc
  src/FourSteelSlabs.cc
  src/OpticalDetector.cc
  src/SegmentedSimpleCmsDetector.cc
  src/SimpleCmsDetector.cc
  src/TestEm3Detector.cc
)

target_compile_features(gdml-gen
  PUBLIC cxx_std_14
)

target_include_directories(gdml-gen PRIVATE
  $<BUILD_INTERFACE:
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(gdml-gen PRIVATE ${Geant4_LIBRARIES})
