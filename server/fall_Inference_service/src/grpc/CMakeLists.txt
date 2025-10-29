target_add_lib(FALL_INFERENCE_SERVICE_GRPC
    FALL_INFERENCE_SERVICE_FALL_MODEL
    RSEC_protos
    gRPC::grpc++
    gRPC::grpc++_reflection
)

target_include_directories(FALL_INFERENCE_SERVICE_GRPC
    BEFORE
    PUBLIC
    $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
)
