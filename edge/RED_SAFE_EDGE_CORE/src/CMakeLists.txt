set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/ipcscan.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_SRCS
    ${GENERATED_DIR}/ipcscan.pb.cc
    ${GENERATED_DIR}/ipcscan.grpc.pb.cc)
set(PROTO_HDRS
    ${GENERATED_DIR}/ipcscan.pb.h
    ${GENERATED_DIR}/ipcscan.grpc.pb.h)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND protobuf::protoc
    ARGS --cpp_out ${GENERATED_DIR}
         --grpc_out ${GENERATED_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating IPCscan protobuf files"
)

add_library(ipcscan_proto STATIC ${PROTO_SRCS})
target_include_directories(ipcscan_proto PUBLIC ${GENERATED_DIR})
target_link_libraries(ipcscan_proto PUBLIC gRPC::grpc++ protobuf::libprotobuf)

include(${PROJECT_SOURCE_DIR}/cmake/Target.cmake)

target_add_lib(RED_SAFE_EDGE_corelib
    ipcscan_proto
    paho-mqtt3as-static
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    gRPC::grpc++
    protobuf::libprotobuf
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Boost::system)

target_include_directories(RED_SAFE_EDGE_corelib
    PUBLIC
    ${GENERATED_DIR}
    ${paho-mqtt-c_SOURCE_DIR}/src
    ${paho-mqtt-c_BINARY_DIR}/src
)

target_add_bin(RED_SAFE_EDGE_CORE main.cc RED_SAFE_EDGE_corelib)
