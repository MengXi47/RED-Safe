cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_DEPLOYMENT_TARGET "26.0" CACHE STRING "" FORCE)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(RED_SAFE_EDGE_CORE VERSION 1.0
    DESCRIPTION "CORE"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (WIN32)
  add_compile_definitions(
      _WIN32_WINNT=0x0601
      WIN32_LEAN_AND_MEAN
  )
endif ()

include(cmake/Target.cmake)

find_package(Boost 1.82 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
if (APPLE)
  find_package(Protobuf CONFIG REQUIRED)
else ()
  find_package(Protobuf REQUIRED)
endif ()
find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED IMPORTED_TARGET libpqxx)

add_subdirectory(src)
