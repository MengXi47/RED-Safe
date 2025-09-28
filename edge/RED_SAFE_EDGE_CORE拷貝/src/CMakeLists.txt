add_subdirectory(common)
add_subdirectory(ipcscan)
add_subdirectory(core)

target_add_bin(RED_SAFE_EDGE_CORE main.cc
    RED_SAFE_EDGE_core
)

target_include_directories(RED_SAFE_EDGE_CORE
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/ipcscan/generated
)

set_target_properties(RED_SAFE_EDGE_CORE PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
