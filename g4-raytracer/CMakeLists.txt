#---------------------------------*-CMake-*----------------------------------#
# Copyright 2024 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
# Geant4 Raytracer app CMakeLists.txt
#----------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.10)
project(g4-raytrace)

cmake_policy(VERSION 3.12...3.16)
set(CMAKE_CXX_STANDARD 17)

#----------------------------------------------------------------------------#
# Find Geant4
find_package(Geant4 REQUIRED vis_all)

#----------------------------------------------------------------------------#
# Add executable
add_executable(main main.cc src/DetectorConstruction.cc)

target_include_directories(main PRIVATE
 ${PROJECT_SOURCE_DIR}/src
 ${Geant4_INCLUDE_DIR}
)

target_link_libraries(main PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------#
# Copy visualization macro
configure_file(${PROJECT_SOURCE_DIR}/vis.mac
  ${PROJECT_BINARY_DIR} COPYONLY
)
