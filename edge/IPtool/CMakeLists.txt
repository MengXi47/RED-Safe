cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(IPtool VERSION 1.0
    DESCRIPTION "IPtool"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(cmake/Target.cmake)

if (APPLE)
  find_package(Protobuf CONFIG REQUIRED)
else ()
  find_package(Protobuf REQUIRED)
endif ()

find_package(gRPC CONFIG REQUIRED)

add_subdirectory(src)