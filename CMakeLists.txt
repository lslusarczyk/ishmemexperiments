# SPDX-FileCopyrightText: Intel Corporation
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22)
project(ishmem_experiments)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
  message(FATAL_ERROR "use intel compiler")
endif()

add_compile_options(-fsycl)
add_link_options(-fsycl)

set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
include(ExternalProject)
include(CTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib)

find_package(MPI REQUIRED)
message("MPI exec set to ${MPIEXEC_EXECUTABLE}")

if(NOT OFI_PROVIDER STREQUAL "verbs" AND NOT OFI_PROVIDER STREQUAL "psm3")
  message(FATAL_ERROR "not set or not supported OFI_PROVIDER:${OFI_PROVIDER}")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(ENABLE_DEBUG_PARAM "--enable-debug")
else()
  set(ENABLE_DEBUG_PARAM "")
endif()

ExternalProject_Add(
  level-zero
  GIT_REPOSITORY https://github.com/oneapi-src/level-zero.git
  GIT_TAG v1.14.0
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
  CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}")

ExternalProject_Add(
  ofi
  GIT_REPOSITORY git@github.com:ofiwg/libfabric.git
  GIT_TAG v1.18.0
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ofisrc
  DEPENDS level-zero
  CONFIGURE_COMMAND
    cd ${CMAKE_CURRENT_BINARY_DIR}/ofisrc && ./autogen.sh && ./configure
    --prefix=${CMAKE_CURRENT_BINARY_DIR} ${ENABLE_DEBUG_PARAM} --enable-only
    --enable-${OFI_PROVIDER}=dl --with-dlopen=yes
  BUILD_IN_SOURCE ON
  BUILD_COMMAND make
  INSTALL_COMMAND make install)

# --enable-hard-polling is special to verbs/psm3
ExternalProject_Add(
  sos
  GIT_REPOSITORY git@github.com:Sandia-OpenSHMEM/SOS.git
  GIT_TAG main
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/sossrc
  DEPENDS level-zero ofi
  CONFIGURE_COMMAND
    cd ${CMAKE_CURRENT_BINARY_DIR}/sossrc && ./autogen.sh && ./configure
    --prefix=${CMAKE_CURRENT_BINARY_DIR} ${ENABLE_DEBUG_PARAM}
    --enable-pmi-simple --with-ofi=${CMAKE_CURRENT_BINARY_DIR} --disable-fortran
    --enable-ofi-mr=basic --disable-ofi-inject --enable-ofi-hmem
    --disable-bounce-buffers --enable-hard-polling
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
    "-DSHMEM_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}"
    "-DL0_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}")

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror>)
add_compile_options(-Wno-tautological-constant-compare)
add_compile_options(-Wno-error=pass-failed)
add_compile_options(-Wno-error=unknown-pragmas)

function(target_link_ishmem exec_name)
  target_link_libraries(${exec_name} ze_loader pmi_simple sma)
  target_link_libraries(${exec_name} ${CMAKE_BINARY_DIR}/lib/libishmem.a)
endfunction()

function(add_ctest name binname nproc)
  set(options OFFLOAD GDB)
  cmake_parse_arguments(ISHMEXP "${options}" "" "" ${ARGN})

  if(ISHMEXP_OFFLOAD)
    set(extra_mpiflags
        "-genv;I_MPI_OFFLOAD;1;-genv;I_MPI_OFFLOAD_LEVEL_ZERO_LIBRARY;${CMAKE_BINARY_DIR}/lib64/libze_loader.so"
    )
  endif()

  if(ISHMEXP_GDB)
    set(wrapper_script
        "gdb-oneapi;-return-child-result;--ex;b abort;--ex;r;-ex;bt;--ex;q;--batch;--args")
    set(extra_mpiflags "${extra_mpiflags};-genv;ZET_ENABLE_PROGRAM_DEBUGGING;1")
  endif()

  # override system set FI_PROVIDER_PATH
  set(extra_mpiflags
      "${extra_mpiflags};-genv;FI_PROVIDER_PATH;${CMAKE_BINARY_DIR}/lib/libfabric;-genv;I_MPI_OFI_LIBRARY;${CMAKE_BINARY_DIR}/lib/libfabric.so;-genv;I_MPI_OFI_PROVIDER;${OFI_PROVIDER};-genv;FI_LOG_LEVEL;Debug;-genv;FI_LOG_SUBSYS;core,fabric,domain,ep_ctrl,ep_data,av,cq,eq,mr;-genv;I_MPI_DEBUG;1000;-genv;SHMEM_DEBUG;1;-genv;FI_PROVIDER;${OFI_PROVIDER}"
  )

  add_test(
    NAME ${name}
    COMMAND
      ${MPIEXEC_EXECUTABLE} -n ${nproc} -outfile-pattern "${name}.%r.out.log"
      -errfile-pattern "${name}.%r.err.log" ${extra_mpiflags}
      ${CMAKE_BINARY_DIR}/bin/ishmrun ${wrapper_script} ./${binname}
      COMMAND_EXPAND_LISTS)
endfunction()

add_executable(ishmexp main.cpp)
# target_compile_definitions(ishmexp PRIVATE USE_MPI)
target_link_libraries(ishmexp MPI::MPI_CXX)
target_link_ishmem(ishmexp)

add_ctest(itest ishmexp 1)
add_ctest(gtest ishmexp 1 GDB)
add_ctest(otest ishmexp 1 OFFLOAD)
# add_ctest(itest-offload ishmexp 1 OFFLOAD)
