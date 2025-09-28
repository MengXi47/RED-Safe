set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/red_safe_edge_core.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRC
    ${PROTO_GEN_DIR}/red_safe_edge_core.pb.cc
    ${PROTO_GEN_DIR}/red_safe_edge_core.grpc.pb.cc
)
set(PROTO_HDR
    ${PROTO_GEN_DIR}/red_safe_edge_core.pb.h
    ${PROTO_GEN_DIR}/red_safe_edge_core.grpc.pb.h
)

add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR}
    COMMAND protobuf::protoc
    ARGS --cpp_out=${PROTO_GEN_DIR}
         --grpc_out=${PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I ${CMAKE_CURRENT_SOURCE_DIR}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating red_safe_edge_core protobuf sources"
)

add_library(RED_SAFE_EDGE_proto STATIC ${PROTO_SRC})

target_include_directories(RED_SAFE_EDGE_proto
    PUBLIC
        ${PROTO_GEN_DIR}
)

target_link_libraries(RED_SAFE_EDGE_proto
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
)

