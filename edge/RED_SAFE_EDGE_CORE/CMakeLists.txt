cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(RED_SAFE_EDGE_CORE VERSION 1.0
    DESCRIPTION "Unified edge client with MQTT, IPC scan, and gRPC"
    LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

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

FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost 1.70 REQUIRED COMPONENTS system)

add_subdirectory(src)
