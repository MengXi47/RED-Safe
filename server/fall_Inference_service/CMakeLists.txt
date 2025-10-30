cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_DEPLOYMENT_TARGET "26.0" CACHE STRING "" FORCE)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project(fall_inference_service VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(cmake/Target.cmake)
include(cmake/CLangFormat.cmake)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir
      OUTPUT_VARIABLE CLANG_RESOURCE_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (CLANG_RESOURCE_DIR)
    get_filename_component(CLANG_PREFIX "${CLANG_RESOURCE_DIR}/../.." ABSOLUTE)
    set(LIBCXX_INCLUDE_DIR "${CLANG_PREFIX}/include/c++/v1")
    if (EXISTS "${LIBCXX_INCLUDE_DIR}")
      include_directories(BEFORE SYSTEM "${LIBCXX_INCLUDE_DIR}")
    endif ()
  endif ()
endif ()

foreach(_implicit_dir IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
  if(_implicit_dir MATCHES "/c\\+\\+/v1(/)?$")
    include_directories(BEFORE SYSTEM "${_implicit_dir}")
  endif()
endforeach()

find_package(folly REQUIRED)

if (TARGET Folly::folly_deps)
  get_target_property(_folly_includes Folly::folly_deps INTERFACE_INCLUDE_DIRECTORIES)
  if (_folly_includes)
    list(FILTER _folly_includes EXCLUDE REGEX "MacOSX[0-9.]+\\.sdk/usr/include")
    set_property(
        TARGET Folly::folly_deps
        PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${_folly_includes}")
  endif ()
  get_target_property(_folly_deps Folly::folly_deps INTERFACE_LINK_LIBRARIES)
  if (_folly_deps)
    find_library(GFLAGS_LIBRARY NAMES gflags)
    if (GFLAGS_LIBRARY)
      list(TRANSFORM _folly_deps REPLACE "gflags_shared" "${GFLAGS_LIBRARY}")
    endif ()
    set_property(
        TARGET Folly::folly_deps
        PROPERTY INTERFACE_LINK_LIBRARIES "${_folly_deps}")
  endif ()
endif ()

find_package(Boost 1.82 REQUIRED COMPONENTS)
find_package(OpenSSL REQUIRED)
if (APPLE)
  find_package(Protobuf CONFIG REQUIRED)
else ()
  find_package(Protobuf REQUIRED)
endif ()
find_package(gRPC CONFIG REQUIRED)

find_package(Torch REQUIRED)

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

message(STATUS "TORCH_INCLUDE_DIRS = ${TORCH_INCLUDE_DIRS}")
message(STATUS "TORCH_LIBRARIES    = ${TORCH_LIBRARIES}")

add_subdirectory(src)
