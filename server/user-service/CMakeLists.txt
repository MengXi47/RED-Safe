target_add_lib(RED_Safe_user_service_lib
    folly
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${nlohmann_json_LIBS}
    ${PQXX_TARGET}
    ${sodium_PKG_TARGET}
    jwt
)
target_add_bin(RED_Safe_user_service user_service_main.cc RED_Safe_user_service_lib)
