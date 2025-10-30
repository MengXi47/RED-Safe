target_add_lib(fall_inference_service_grpc
    fall_inference_service_fall_model
    RSEC_protos
    gRPC::grpc++
    gRPC::grpc++_reflection
)

target_include_directories(fall_inference_service_grpc
    BEFORE
    PUBLIC
    $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
)
