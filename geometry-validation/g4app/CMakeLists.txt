#---------------------------------*-CMake-*----------------------------------#
# Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
project(g4-geo-validation-app)

cmake_minimum_required(VERSION 3.9)
cmake_policy(VERSION 3.12...3.16)
set(CMAKE_CXX_STANDARD 17)

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
find_package(Geant4 REQUIRED vis_all)

#----------------------------------------------------------------------------#
# Locate sources and headers of the project
#----------------------------------------------------------------------------#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.hh)

#----------------------------------------------------------------------------#
# Add executable
#----------------------------------------------------------------------------#
add_executable(g4app g4app.cc ${sources} ${headers})

target_include_directories(g4app PRIVATE
 ${PROJECT_SOURCE_DIR}/src
 ${Geant4_INCLUDE_DIR}
)

target_link_libraries(g4app PRIVATE
  ${Geant4_LIBRARIES}
)

#----------------------------------------------------------------------------#
# Install the executable in the current directory
#----------------------------------------------------------------------------#
install(TARGETS g4app DESTINATION .)

file(COPY 
  ${PROJECT_SOURCE_DIR}/compare_geometries.sh
  DESTINATION
  ${CMAKE_CURRENT_BINARY_DIR}
)
