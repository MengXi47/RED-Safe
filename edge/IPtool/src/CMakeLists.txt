set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")

file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

foreach (PROTO_FILE ${PROTO_FILES})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

  set(PB_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
  set(PB_H "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
  set(GRPC_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
  set(GRPC_H "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

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
endforeach ()

set_source_files_properties(${GENERATED_SOURCES} ${GENERATED_HEADERS} PROPERTIES GENERATED TRUE)

add_library(iptool_protos STATIC ${GENERATED_SOURCES} ${GENERATED_HEADERS})

target_include_directories(iptool_protos
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(iptool_protos
    PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc
)

target_add_lib(IPtool_lib iptool_protos)

target_add_bin(IPtool_bin main.cc IPtool_lib)
