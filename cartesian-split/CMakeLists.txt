cmake_minimum_required(VERSION 3.19...3.23 FATAL_ERROR)

set(TARGET "stencil")
set(TARGET_VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(${TARGET} LANGUAGES CXX VERSION ${TARGET_VERSION})


if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
        message("CMAKE_BUILD_TYPE is " ${CMAKE_BUILD_TYPE})
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "CXX = \"$ENV{CXX}\" of ${TARGET}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wunused-parameter -Wextra -Wreorder -Wconversion -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast -Wunused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-slp-vectorize -ftree-vectorize")



get_filename_component(HOME_DIR ${PROJECT_SOURCE_DIR} PATH)
set(WORKDIR ${CMAKE_CURRENT_LIST_DIR})
set(INCLUDE_DIR ${HOME_DIR}/include)
message(STATUS "INCLUDE_DIR in: ${INCLUDE_DIR}")
message(STATUS "WORKDIR in: ${WORKDIR}")
message(STATUS "PROJECT_SOURCE_DIR in: ${PROJECT_SOURCE_DIR}")

include_directories(${INCLUDE_DIR})
file(GLOB SRC
    "${WORKDIR}/*.cc"
)

# include(FetchContent)
# FetchContent_Declare(
#   addlib
#   URL ${HOME_DIR}/cuda
# )
# FetchContent_MakeAvailable(addlib)

add_executable(
  ${TARGET}
  ${SRC}
)

set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
target_compile_features(${TARGET} PRIVATE cxx_std_${CMAKE_CXX_STANDARD})

set_target_properties(
    ${TARGET}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(
    ${TARGET}
    PRIVATE
    ${MPI_C_LIBRARIES} 
    ${MPI_CXX_LIBRARIES}
    # thrustlib
)
