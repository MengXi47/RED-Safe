set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/ipcscan.proto)

set(GENERATED_SOURCES)
set(GENERATED_HEADERS)

foreach(PROTO_FILE ${PROTO_FILES})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
  set(PB_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
  set(PB_H "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
  set(GRPC_CC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
  set(GRPC_H "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")

  add_custom_command(
      OUTPUT ${PB_CC} ${PB_H}
      COMMAND protobuf::protoc
      ARGS --cpp_out ${CMAKE_CURRENT_BINARY_DIR}
           -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
           ${PROTO_FILE}
      DEPENDS ${PROTO_FILE}
      COMMENT "Generating protobuf sources for ${PROTO_NAME}"
  )

  add_custom_command(
      OUTPUT ${GRPC_CC} ${GRPC_H}
      COMMAND protobuf::protoc
      ARGS --grpc_out ${CMAKE_CURRENT_BINARY_DIR}
           --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
           -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
           ${PROTO_FILE}
      DEPENDS ${PROTO_FILE}
      COMMENT "Generating gRPC sources for ${PROTO_NAME}"
  )

  list(APPEND GENERATED_SOURCES ${PB_CC} ${GRPC_CC})
  list(APPEND GENERATED_HEADERS ${PB_H} ${GRPC_H})
endforeach()

add_library(ipcscan_proto STATIC ${GENERATED_SOURCES})
target_link_libraries(ipcscan_proto
    PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
)
target_include_directories(ipcscan_proto
    PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_add_lib(IPCscan_lib
    ${Boost_LIBRARIES}
    ipcscan_proto
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_add_bin(IPCscan_bin main.cc IPCscan_lib)
