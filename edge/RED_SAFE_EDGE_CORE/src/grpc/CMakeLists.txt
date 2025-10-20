target_add_lib(RED_SAFE_EDGE_CORE_grpc
    RED_SAFE_EDGE_UTIL
    RED_SAFE_EDGE_CORE_ipcscan
    RSEC_protos
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)
