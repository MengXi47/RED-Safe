target_add_lib(RED_Safe_ios_service_lib
    folly
    redsafeserver_http
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${nlohmann_json_LIBS}
    ${PQXX_TARGET}
    ${sodium_PKG_TARGET}
    jwt
    gRPC::grpc++
    redsafe_proto
)
target_add_bin(RED_Safe_ios_service ios_service_main.cc RED_Safe_ios_service_lib)

