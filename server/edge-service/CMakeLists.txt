target_add_lib(RED_Safe_edge_service_lib
    folly
    ${Boost_LIBRARIES}
    ${nlohmann_json_LIBS}
    ${PQXX_TARGET}
)
target_add_bin(RED_Safe_edge_service edge_service_main.cc RED_Safe_edge_service_lib)
