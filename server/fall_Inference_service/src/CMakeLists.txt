add_subdirectory(fall_model)
add_subdirectory(grpc)

target_add_bin(FALL_INFERENCE_SERVICE_BIN mian.cc
    FALL_INFERENCE_SERVICE_FALL_MODEL
    FALL_INFERENCE_SERVICE_GRPC
    RSEC_protos
    Folly::folly
    gRPC::grpc++
    gRPC::grpc++_reflection
)

target_include_directories(FALL_INFERENCE_SERVICE_BIN
    BEFORE
    PRIVATE
    $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
)
