#---------------------------------*-CMake-*----------------------------------#
# Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
# See the Celeritas top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#
project(vg-app)

cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 11)

#----------------------------------------------------------------------------#
# Find packages
#----------------------------------------------------------------------------#
find_package(XercesC REQUIRED)
find_package(VecGeom REQUIRED)

#----------------------------------------------------------------------------#
# Add executable
#----------------------------------------------------------------------------#
add_executable(vgapp vgapp.cc)

target_include_directories(vgapp PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${VecGeom_INCLUDE_DIRS}
  ${XercesC_INCLUDE_DIR}
)

target_link_libraries(vgapp PRIVATE
  VecGeom::vecgeom
  VecGeom::vgdml
  XercesC::XercesC
)

#----------------------------------------------------------------------------#
# Install the executable in the current directory
#----------------------------------------------------------------------------#
install(TARGETS vgapp DESTINATION .)
