target_add_lib(RED_SAFE_EDGE_network
    RED_SAFE_EDGE_proto
    RED_SAFE_EDGE_common
    Boost::system
    gRPC::grpc++
    protobuf::libprotobuf
)

target_include_directories(RED_SAFE_EDGE_network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

