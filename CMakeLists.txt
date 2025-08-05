cmake_minimum_required(VERSION 3.16)

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(RED_Safe VERSION 1.0
    DESCRIPTION "RED-Safe"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")


message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (APPLE)
  message(STATUS "Building on macOS")
elseif (WIN32)
  message(STATUS "Building on Windows")
  add_compile_definitions(
      _WIN32_WINNT=0x0601
      WIN32_LEAN_AND_MEAN
  )
elseif (UNIX)
  message(STATUS "Building on Linux/Unix")
endif ()

include(cmake/Target.cmake)
list(APPEND CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmake"
)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(libpqxx CONFIG QUIET)
find_package(libsodium CONFIG QUIET)

if (TARGET libpqxx::pqxx)
  set(PQXX_TARGET libpqxx::pqxx)
else ()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PQXX REQUIRED libpqxx)
  add_library(pqxx INTERFACE)
  target_include_directories(pqxx INTERFACE ${PQXX_INCLUDE_DIRS})
  target_link_libraries(pqxx INTERFACE ${PQXX_LINK_LIBRARIES})
  set(PQXX_TARGET pqxx)
endif ()

add_subdirectory("third_party/folly" EXCLUDE_FROM_ALL)
add_subdirectory(server)
