cmake_minimum_required(VERSION 3.15)

project(RED_Safe_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-------------------- 來源檔 -------------------
add_executable(RED_Safe_server
    src/main.cpp
    src/api-server.cpp
    src/session.cpp
)

if(APPLE)
  message(STATUS "Building on macOS")
  target_compile_definitions(RED_Safe_server PRIVATE PLATFORM_MACOS)
elseif(WIN32)
  message(STATUS "Building on Windows")
  add_compile_definitions(_WIN32_WINNT=0x0601)
  target_compile_definitions(RED_Safe_server PRIVATE PLATFORM_WINDOWS)
elseif(UNIX)
  message(STATUS "Building on Linux/Unix")
  target_compile_definitions(RED_Safe_server PRIVATE PLATFORM_UNIX)
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

#-------------------- 依賴 --------------------
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

target_include_directories(RED_Safe_server
    PRIVATE
      ${Boost_INCLUDE_DIRS}
      ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(RED_Safe_server
    PRIVATE
      ${Boost_LIBRARIES}
      OpenSSL::SSL
      OpenSSL::Crypto
)

# target_link_libraries(RED_Safe_server
#     PRIVATE
#       Boost::asio
#       Boost::system
#       Boost::thread
#       OpenSSL::SSL
#       OpenSSL::Crypto
# )

#-------------------- 複製憑證 ------------------
configure_file(${CMAKE_SOURCE_DIR}/server.crt  server.crt  COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/server.key  server.key  COPYONLY)
