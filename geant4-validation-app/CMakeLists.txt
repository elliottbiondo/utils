#---------------------------------*-CMake-*----------------------------------#
# Copyright 2021-2023 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.9)
project(g4-app)

cmake_policy(VERSION 3.12...3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# For debugging purposes
if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  message(STATUS "Set default CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

#----------------------------------------------------------------------------#
# Build options
#----------------------------------------------------------------------------#
option(USE_ROOT "ROOT I/O" ON)
option(USE_MT "Multithread support" OFF)
option(USE_QT "Qt support" OFF)

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
if(USE_ROOT)
  find_package(ROOT REQUIRED)
  add_compile_definitions(G4APP_USE_ROOT=1)
endif()

if(USE_MT)
  set(_g4_mt multithreaded)
  add_compile_definitions(G4APP_USE_MT=1)
endif()

if (USE_QT)
  set(_g4_qt vis_all)
  add_compile_definitions(G4APP_USE_QT=1)
endif()

find_package(Geant4 REQUIRED gdml ${_g4_mt} ${_g4_qt})
find_package(nlohmann_json REQUIRED)
find_package(HepMC3 REQUIRED)
find_package(Celeritas REQUIRED)
include(CeleritasLibrary)

message(STATUS "Use ROOT: ${USE_ROOT}")
message(STATUS "Use multithread: ${USE_MT}")
message(STATUS "Use Qt: ${USE_QT}")

#----------------------------------------------------------------------------#
# Generate ROOT dictionary
#----------------------------------------------------------------------------#
if(USE_ROOT)
  include(${ROOT_USE_FILE})

  root_generate_dictionary(RootInterface
    ${PROJECT_SOURCE_DIR}/src/RootData.hh
    MODULE rootdata
    LINKDEF ${PROJECT_SOURCE_DIR}/src/RootInterfaceLinkDef.hh
  )

  add_library(rootdata SHARED
    ${PROJECT_SOURCE_DIR}/src/RootData.hh RootInterface.cxx
  )

  target_link_libraries(rootdata PUBLIC ${ROOT_LIBRARIES})
endif()

#----------------------------------------------------------------------------#
# Sources
#----------------------------------------------------------------------------#
set(_src
  src/ActionInitialization.cc
  src/BremsstrahlungProcess.cc
  src/Celeritas.cc
  src/DetectorConstruction.cc
  src/EventAction.cc
  src/Geant4Run.cc
  src/HepMC3Reader.cc
  src/JsonReader.cc
  src/PhysicsList.cc
  src/PrimaryGeneratorAction.cc
  src/RunAction.cc
  src/SensitiveDetector.cc
  src/SteppingAction.cc
  src/Stopwatch.cc
  src/TrackingAction.cc
)

if(USE_ROOT)
  list(APPEND _src
    src/RootIO.root.cc
    src/RootUniquePtr.root.cc
  )
endif()

#----------------------------------------------------------------------------#
# Includes and libraries
#----------------------------------------------------------------------------#
set(_includes
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${Geant4_INCLUDE_DIR}
  ${NLOHMANN_SOURCE_DIR}/include
  ${HEPMC3_INCLUDE_DIR}
  ${Celeritas_ROOT}/include
)

set(_libs
  ${Geant4_LIBRARIES}
  ${HEPMC3_LIB}
  Celeritas::celeritas
  Celeritas::accel
  nlohmann_json::nlohmann_json
)

if(USE_ROOT)
  list(APPEND _includes ${ROOT_INCLUDE_DIR})
  list(APPEND _libs ROOT::Core rootdata)
endif()

#----------------------------------------------------------------------------#
# Executable
#----------------------------------------------------------------------------#
add_executable(g4app g4-app.cc ${_src})
target_include_directories(g4app PRIVATE $<BUILD_INTERFACE:${_includes}>)
celeritas_target_link_libraries(g4app ${_libs})

#----------------------------------------------------------------------------#
# Copy input_example.json and vis.mac to the build directory
#----------------------------------------------------------------------------#
configure_file(${PROJECT_SOURCE_DIR}/input_example.json
  ${PROJECT_BINARY_DIR} COPYONLY
)

if(USE_QT)
  configure_file(${PROJECT_SOURCE_DIR}/vis.mac ${PROJECT_BINARY_DIR} COPYONLY)
endif()
