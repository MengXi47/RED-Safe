target_add_lib(RED_SAFE_EDGE_IPCSCAN
    RED_SAFE_EDGE_proto
    RED_SAFE_EDGE_network
    Boost::system
    nlohmann_json::nlohmann_json
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

target_include_directories(RED_SAFE_EDGE_IPCSCAN
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
