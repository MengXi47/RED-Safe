include("${Protobuf_DIR}/protobuf-generate.cmake")

add_library(redsafe_proto)

protobuf_generate(
    TARGET   redsafe_proto
    LANGUAGE cpp
    PROTOS   auth.proto
)

# --- Generate gRPC C++ stubs ---
set(GRPC_CC  ${CMAKE_CURRENT_BINARY_DIR}/auth.grpc.pb.cc)
set(GRPC_H   ${CMAKE_CURRENT_BINARY_DIR}/auth.grpc.pb.h)

add_custom_command(
    OUTPUT  ${GRPC_CC} ${GRPC_H}
    COMMAND protobuf::protoc
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/auth.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/auth.proto
    COMMENT "Generate gRPC C++ sources for auth.proto"
    VERBATIM
)

target_sources(redsafe_proto
    PRIVATE
        ${GRPC_CC}
        ${GRPC_H}
)

target_link_libraries(redsafe_proto PUBLIC protobuf::libprotobuf gRPC::grpc++_reflection)
target_include_directories(redsafe_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
