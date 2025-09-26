cmake_minimum_required(VERSION 3.16)

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(RED_Safe VERSION 1.0
    DESCRIPTION "MQTTservice"
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

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

set(PAHO_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PAHO_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(PAHO_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(PAHO_WITH_SSL ON CACHE BOOL "" FORCE)
set(PAHO_ENABLE_WEBSOCKET ON CACHE BOOL "" FORCE)
FetchContent_Declare(paho-mqtt-c
    URL https://github.com/eclipse/paho.mqtt.c/archive/refs/tags/v1.3.13.tar.gz
)
FetchContent_MakeAvailable(paho-mqtt-c)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(CURL REQUIRED)

add_subdirectory(src)
