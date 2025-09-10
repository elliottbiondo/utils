#------------------------------- -*- cmake -*- -------------------------------#
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#-----------------------------------------------------------------------------#
# Celeritas-Geant4 offload application for physics validation
#-----------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.18)
project(celer-geant-validation VERSION 0.0.1 LANGUAGES CXX)
cmake_policy(VERSION 3.12...3.30)

# For debugging purposes
if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
  message(STATUS "Set default CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

#-----------------------------------------------------------------------------#
# Find packages
find_package(Celeritas 0.6 REQUIRED)
find_package(Geant4 REQUIRED gdml multithreaded) # TODO: check minimum version
find_package(ROOT REQUIRED)
find_package(HepMC3 REQUIRED)
find_package(nlohmann_json REQUIRED)

if(NOT Celeritas_FOUND)
  message(FATAL_ERROR
    "No Celeritas installation found.\n"
    "Export Celeritas_ROOT=/path/to/celeritas/install or use CMAKE_INSTALL_PREFIX")
endif()

if(NOT CELERITAS_USE_Geant4)
  message(SEND_ERROR "This example requires Geant4 support "
    "to be enabled in Celeritas")
endif()

#-----------------------------------------------------------------------------#
# Add executable
set(SOURCES
  src/ActionInitialization.cc
  src/DetectorConstruction.cc
  src/EventAction.cc
  src/JsonReader.cc
  src/PrimaryGeneratorAction.cc
  src/RootDataStore.cc
  src/RootIO.cc
  src/RunAction.cc
  src/SensitiveDetector.cc
  src/StackingAction.cc
)

add_executable(celer-geant
  celer-geant.cc
  ${SOURCES}
)

target_include_directories(celer-geant PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

celeritas_target_link_libraries(celer-geant
  Celeritas::celeritas
  Celeritas::G4
  ${Geant4_LIBRARIES}
  ROOT::Tree
  ROOT::Hist
  HepMC3::HepMC3
  nlohmann_json::nlohmann_json
)
