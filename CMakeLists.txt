# SPDX-FileCopyrightText: Intel Corporation
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22)
project(ishmem_experiments)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
  message(FATAL_ERROR "use intel compiler")
endif()

include(FetchContent)
include(ExternalProject)
include(CTest)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(ENABLE_DEBUG_PARAM "--enable-debug")
else()
  set(ENABLE_DEBUG_PARAM "")
endif()

# --enable-ofi-manual-progress --enable-mr-endpoint is special to cxi
ExternalProject_Add(
  sos
  GIT_REPOSITORY git@github.com:Sandia-OpenSHMEM/SOS.git
  GIT_TAG main
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/sossrc
  CONFIGURE_COMMAND
    cd ${CMAKE_CURRENT_BINARY_DIR}/sossrc && ./autogen.sh && ./configure
    --prefix=${CMAKE_CURRENT_BINARY_DIR} ${ENABLE_DEBUG_PARAM}
    --enable-pmi-simple --with-ofi=/opt/cray/libfabric/1.15.2.0 --disable-fortran
    --enable-ofi-mr=basic --disable-ofi-inject --enable-ofi-hmem
    --disable-bounce-buffers --enable-ofi-manual-progress --enable-mr-endpoint
  BUILD_IN_SOURCE ON
  BUILD_COMMAND make
  INSTALL_COMMAND make install)

ExternalProject_Add(
  ishmem
  GIT_REPOSITORY
    git@github.com:intel-innersource/libraries.runtimes.hpc.shmem.ishmem.git
  GIT_TAG main
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS sos
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=STRING:${CMAKE_BUILD_TYPE}
  CMAKE_CACHE_ARGS
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}"
    "-DSHMEM_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}")
