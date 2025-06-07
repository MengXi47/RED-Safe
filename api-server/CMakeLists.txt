cmake_minimum_required(VERSION 3.16)

project(RED_Safe_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-------------------- 來源檔 -------------------
add_executable(RED_Safe_apiserver
    main.cpp
    src/core/api-server.cpp
    src/core/session.cpp
    src/core/controller.cpp
    src/service/EdgeService.cpp
    src/service/IOSAPPService.cpp
    src/service/UserService.cpp
    src/service/TokenService.cpp
    src/model/sql/write.cpp
    src/model/sql/read.cpp
)

if(APPLE)
    message(STATUS "Building on macOS")
    target_compile_definitions(RED_Safe_apiserver PRIVATE PLATFORM_MACOS)
elseif(WIN32)
    message(STATUS "Building on Windows")
    add_compile_definitions(
        _WIN32_WINNT=0x0601
        WIN32_LEAN_AND_MEAN
    )
    target_compile_definitions(RED_Safe_apiserver PRIVATE PLATFORM_WINDOWS)
    find_package(libsodium CONFIG REQUIRED)
elseif(UNIX)
    message(STATUS "Building on Linux/Unix")
    target_compile_definitions(RED_Safe_apiserver PRIVATE PLATFORM_UNIX)
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

#-------------------- 依賴 --------------------
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libpqxx CONFIG QUIET)
find_package(libsodium CONFIG QUIET)

if (TARGET libpqxx::pqxx)                       # 成功找到
    set(PQXX_TARGET libpqxx::pqxx)
else ()                                         # 回退到 pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED libpqxx)
    add_library(pqxx INTERFACE)
    target_include_directories(pqxx INTERFACE ${PQXX_INCLUDE_DIRS})
    target_link_libraries(pqxx INTERFACE ${PQXX_LINK_LIBRARIES})
    set(PQXX_TARGET pqxx)
endif()

if (TARGET libsodium::libsodium)                       # 成功找到
    set(sodium_PKG_TARGET libsodium::libsodium)
else ()                                         # 回退到 pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(sodium_PKG QUIET libsodium)
    add_library(sodium_PKG INTERFACE)
    target_include_directories(sodium_PKG INTERFACE ${sodium_PKG_INCLUDE_DIRS})
    target_link_libraries(sodium_PKG INTERFACE ${sodium_PKG_LINK_LIBRARIES})
    set(sodium_PKG_TARGET sodium_PKG)
endif()

target_include_directories(RED_Safe_apiserver
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${nlohmann_json_INCLUDE_DIRS}
        folly
)

target_link_libraries(RED_Safe_apiserver
    PRIVATE
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${nlohmann_json_LIBS}
        ${PQXX_TARGET}
        ${sodium_PKG_TARGET}
        folly
        ${GFLAGS_LIBRARIES}
)
