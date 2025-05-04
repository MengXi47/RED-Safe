# api-server/CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(RED_Safe_API_Server LANGUAGES CXX)

# ——— 全域 C++20／關閉 compiler extensions ———
set(CMAKE_CXX_STANDARD        20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS       OFF)

# 針對新版 CMake 找不到舊 FindBoost module 的 policy
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(RED_Safe_API_Server LANGUAGES CXX)

# ——— 找 Boost.Asio 所需的模組 ———
find_package(Boost REQUIRED COMPONENTS system thread)
# ——— 找 OpenSSL 所需的模組 ———
find_package(OpenSSL REQUIRED)

# ——— 建立可執行檔 ———
add_executable(RED_Safe_server
  src/main.cpp
  src/server.cpp
  src/session.cpp
)

# ——— Include 路徑 & Link 函式庫 ———
# PRIVATE：只對本 target 生效，不會傳遞給其他 target
target_include_directories(RED_Safe_server PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(RED_Safe_server PRIVATE
  ${Boost_LIBRARIES}
  OpenSSL::SSL
  OpenSSL::Crypto
)

add_custom_command(TARGET RED_Safe_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/server.crt
            ${CMAKE_SOURCE_DIR}/server.key
            $<TARGET_FILE_DIR:RED_Safe_server>
)

# ——— 不同平台／編譯器的額外定義或 flags ———
if(APPLE)
  message(STATUS "Building on macOS")
  target_compile_definitions(RED_Safe_server PRIVATE PLATFORM_MACOS)
elseif(WIN32)
  message(STATUS "Building on Windows")
  target_compile_definitions(RED_Safe_server PRIVATE PLATFORM_WINDOWS)
elseif(UNIX)
  message(STATUS "Building on Linux/Unix")
  target_compile_definitions(RED_Safe_server PRIVATE PLATFORM_UNIX)
endif()
