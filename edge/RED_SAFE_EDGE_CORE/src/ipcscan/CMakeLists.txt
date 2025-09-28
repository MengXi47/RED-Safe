set(IPC_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(IPC_PROTO_FILE ${IPC_PROTO_DIR}/ipcscan.proto)
set(IPC_PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${IPC_PROTO_GEN_DIR})

set(IPC_PROTO_SRC
    ${IPC_PROTO_GEN_DIR}/ipcscan.pb.cc
    ${IPC_PROTO_GEN_DIR}/ipcscan.grpc.pb.cc
)
set(IPC_PROTO_HDR
    ${IPC_PROTO_GEN_DIR}/ipcscan.pb.h
    ${IPC_PROTO_GEN_DIR}/ipcscan.grpc.pb.h
)

add_custom_command(
    OUTPUT ${IPC_PROTO_SRC} ${IPC_PROTO_HDR}
    COMMAND protobuf::protoc
    ARGS --cpp_out=${IPC_PROTO_GEN_DIR}
         --grpc_out=${IPC_PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I ${IPC_PROTO_DIR}
         ${IPC_PROTO_FILE}
    DEPENDS ${IPC_PROTO_FILE}
    COMMENT "Generating IPCscan protobuf sources"
)

add_library(RED_SAFE_EDGE_ipcscan_proto STATIC ${IPC_PROTO_SRC})
target_include_directories(RED_SAFE_EDGE_ipcscan_proto
    PUBLIC
        ${IPC_PROTO_GEN_DIR}
)
target_link_libraries(RED_SAFE_EDGE_ipcscan_proto
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
)

target_add_lib(RED_SAFE_EDGE_IPCSCAN
    RED_SAFE_EDGE_ipcscan_proto
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    Boost::system
    nlohmann_json::nlohmann_json
)

add_dependencies(RED_SAFE_EDGE_IPCSCAN RED_SAFE_EDGE_ipcscan_proto)

target_sources(RED_SAFE_EDGE_IPCSCAN PRIVATE ${IPC_PROTO_SRC})

target_include_directories(RED_SAFE_EDGE_IPCSCAN
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${IPC_PROTO_GEN_DIR}
)
