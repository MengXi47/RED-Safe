cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_DEPLOYMENT_TARGET "26.0" CACHE STRING "" FORCE)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(RED_SAFE_EDGE_CORE VERSION 1.0
    DESCRIPTION "CORE"
    LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(cmake/Target.cmake)
include(cmake/CLangFormat.cmake)

find_package(folly REQUIRED)
find_package(Boost 1.82 REQUIRED COMPONENTS)
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

set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")

file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

foreach(PROTO_FILE ${PROTO_FILES})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

  set(PB_CC   "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
  set(PB_H    "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
  set(GRPC_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
  set(GRPC_H  "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

  add_custom_command(
      OUTPUT ${PB_CC} ${PB_H}
      COMMAND protobuf::protoc
      ARGS
      --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
      -I "${PROTO_DIR}"
      "${PROTO_FILE}"
      DEPENDS "${PROTO_FILE}"
      COMMENT "Generating protobuf sources for ${PROTO_NAME}"
      VERBATIM
  )

  add_custom_command(
      OUTPUT ${GRPC_CC} ${GRPC_H}
      COMMAND protobuf::protoc
      ARGS
      --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
      --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
      -I "${PROTO_DIR}"
      "${PROTO_FILE}"
      DEPENDS "${PROTO_FILE}"
      COMMENT "Generating gRPC sources for ${PROTO_NAME}"
      VERBATIM
  )

  list(APPEND GENERATED_SOURCES ${PB_CC} ${GRPC_CC})
  list(APPEND GENERATED_HEADERS ${PB_H} ${GRPC_H})
endforeach()

set_source_files_properties(${GENERATED_SOURCES} ${GENERATED_HEADERS} PROPERTIES GENERATED TRUE)

add_library(RSEC_protos STATIC ${GENERATED_SOURCES} ${GENERATED_HEADERS})

target_include_directories(RSEC_protos
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(RSEC_protos
    PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc
)

add_subdirectory(src)
