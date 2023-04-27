cmake_minimum_required(VERSION 3.19...3.23 FATAL_ERROR)

set(TARGET "stencil")
set(TARGET_VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(${TARGET} LANGUAGES CXX VERSION ${TARGET_VERSION})

# https://llvm.org/docs/CMake.html#frequently-used-cmake-variables
# 1. Release: `-O3 -DNDEBUG`
# 2. Debug: `-O0 -g`
# 3. RelWithDebInfo: `-O2 -g -DNDEBUG`
# 4. MinSizeRel: `-Os -DNDEBUG`
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# Not print debug info
# add_compile_definitions(NDEBUG=True)
# RelWithDebInfo
if(PRINT_PERF)
    add_compile_definitions(PRINT_PERF=True)
    add_compile_definitions(PERF_ON_RANK_0_ONLY=True)
endif()

message(STATUS "CMAKE_BUILD_TYPE is " ${CMAKE_BUILD_TYPE})
message(STATUS "CXX = \"$ENV{CXX}\" for ${TARGET}")

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

string(APPEND CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -ffast-math -mavx2 -ftree-slp-vectorize -ftree-vectorize -ffast-math -funroll-loops")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast -Wunused-variable-Wunused-parameter -Wextra -Wreorder -Wconversion -Wsign-conversion")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

get_filename_component(HOME_DIR ${PROJECT_SOURCE_DIR} PATH)
set(WORKDIR ${CMAKE_CURRENT_LIST_DIR})
set(INCLUDE_DIR ${WORKDIR}/include)
message(STATUS "CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "PROJECT_SOURCE_DIR in: ${PROJECT_SOURCE_DIR}")
message(STATUS "WORKDIR in: ${WORKDIR}")
message(STATUS "INCLUDE_DIR in: ${INCLUDE_DIR}")
include_directories(${INCLUDE_DIR})

file(GLOB 
    SRC
    "${WORKDIR}/*.cc"
)

add_executable(
  ${TARGET}
  ${SRC}
)
# ------------- DEPENDENCIES ------------- #

include(FetchContent)

find_package(Boost 1.70.0 COMPONENTS program_options)
if(NOT ${Boost_FOUND})
# https://cmake.org/cmake/help/latest/module/FindBoost.html?highlight=findboost
  message(STATUS "Downloading Boost")
  FetchContent_Declare(
    boost_src
    SOURCE_DIR Boost
    # URL ${HOME_DIR}/boost_1_79_0.tar.bz2
    # URL ${HOME_DIR}/boost_1_79_0.zip
    URL https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.bz2
    URL_HASH SHA256=475d589d51a7f8b3ba2ba4eda022b170e562ca3b760ee922c146b6c65856ef39
  )
  FetchContent_MakeAvailable(boost_src)
  message(STATUS "boost_src_SOURCE_DIR in: ${boost_src_SOURCE_DIR}")
  message("${boost_src_SOURCE_DIR}/stage")
  if(NOT EXISTS "${boost_src_SOURCE_DIR}/stage")
    message(STATUS "Building Boost")
    add_custom_command(
      OUTPUT  boost_bootstrap
      COMMAND ${boost_src_SOURCE_DIR}/bootstrap.sh --with-libraries=program_options
      WORKING_DIRECTORY "${boost_src_SOURCE_DIR}"
      COMMENT "bootstrap boost"
    )
    add_custom_target(boost_b2
      COMMAND ${boost_src_SOURCE_DIR}/b2
      WORKING_DIRECTORY "${boost_src_SOURCE_DIR}"
      COMMENT "b2"
      DEPENDS boost_bootstrap
    )
    add_dependencies(${TARGET} boost_b2)
    set(Boost_INCLUDE_DIR ${boost_src_SOURCE_DIR})
    message(${Boost_INCLUDE_DIR})
    set(Boost_INCLUDE_DIRS ${boost_src_SOURCE_DIR})
    message(STATUS "Boost_INCLUDE_DIRS set to ${Boost_INCLUDE_DIRS}")
    set(BOOST_LIBS_OPTIONAL
      program_options
      CACHE STRING "Boost libs to be compiled"
    )
    set(Boost_LIBRARIES "")
    foreach(lib ${BOOST_LIBS_OPTIONAL})
      # In case only a subset of modules is available (eg. after using bcp)
      list(APPEND Boost_LIBRARIES "${boost_src_SOURCE_DIR}/stage/lib/libboost_${lib}.a")
      list(APPEND Boost_LIBRARIES "${boost_src_SOURCE_DIR}/stage/lib/libboost_${lib}.so")
    endforeach()
  else(NOT EXISTS "${boost_src_SOURCE_DIR}/stage")
    set(Boost_INCLUDE_DIR "${CMAKE_BINARY_DIR}/Boost")
  endif(NOT EXISTS "${boost_src_SOURCE_DIR}/stage")
endif(NOT ${Boost_FOUND})

set(Boost_USE_STATIC_LIBS       ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS      ON)  # only find release libs
set(Boost_USE_MULTITHREADED     ON)
set(Boost_USE_STATIC_RUNTIME    ON)

if(EXISTS "${boost_src_SOURCE_DIR}/stage")
  find_package(Boost EXACT 1.79.0 REQUIRED COMPONENTS program_options)
endif(EXISTS "${boost_src_SOURCE_DIR}/stage")
include_directories(${Boost_INCLUDE_DIRS})

# target_include_directories(${TARGET} PUBLIC ${Boost_INCLUDE_DIRS})

set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
target_compile_features(${TARGET} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})

set_target_properties(
    ${TARGET}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
      target_link_libraries(${TARGET} PRIVATE OpenMP::OpenMP_CXX)
  endif()
endif()

target_link_libraries(
    ${TARGET}
    PRIVATE
    ${MPI_C_LIBRARIES} 
    ${MPI_CXX_LIBRARIES}
    ${Boost_LIBRARIES} # Boost::program_options
    # https://github.com/open-mpi/hwloc
    hwloc
    # ibverbs # infiniband
)
