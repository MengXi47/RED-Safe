cmake_minimum_required(VERSION 3.16)
# ── 預設 Release ──────────────────────────────────
if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

project(RED_Safe VERSION 1.0
                 DESCRIPTION "RED-Safe"
                 LANGUAGES CXX)

# ── C++20 ────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── 預設 Release ──────────────────────────────────
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
             "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ── 子專案 ────────────────────────────────────────
add_subdirectory("third_party/folly" EXCLUDE_FROM_ALL)

add_subdirectory(api-server)
