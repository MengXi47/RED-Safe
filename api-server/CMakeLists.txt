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
if(NOT Boost_FOUND)
  message(FATAL_ERROR "找不到 Boost 函式庫，請確認已安裝並設定 BOOST_ROOT")
endif()

# ——— 建立可執行檔 ———
add_executable(RED_Safe_server
  src/main.cpp
  src/server.cpp
)

# ——— Include 路徑 & Link 函式庫 ———
# PRIVATE：只對本 target 生效，不會傳遞給其他 target
target_include_directories(RED_Safe_server PRIVATE
  ${Boost_INCLUDE_DIRS}
)
target_link_libraries(RED_Safe_server PRIVATE
  ${Boost_LIBRARIES}
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
