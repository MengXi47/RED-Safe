protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS auth.proto)
add_library(redsafe_proto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(redsafe_proto PUBLIC protobuf::libprotobuf gRPC::grpc++_reflection)
target_include_directories(redsafe_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
