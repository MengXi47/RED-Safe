add_subdirectory(fall_model)
add_subdirectory(grpc)

target_add_bin(fall_inference_service_bin mian.cc
    fall_inference_service_fall_model
    fall_inference_service_grpc
    RSEC_protos
    Folly::folly
    gRPC::grpc++
    gRPC::grpc++_reflection
)

target_include_directories(fall_inference_service_bin
    BEFORE
    PRIVATE
    $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
)
