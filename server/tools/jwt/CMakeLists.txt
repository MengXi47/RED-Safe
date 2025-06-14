add_library(jwt INTERFACE)
target_include_directories(jwt INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/jwt-cpp
)
target_link_libraries(jwt INTERFACE OpenSSL::Crypto)