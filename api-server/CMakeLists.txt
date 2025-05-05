cmake_minimum_required(VERSION 3.15)

project(RED_Safe_API_Server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)     # 或改 NEW，視你想採用的作法
endif()

#-------------------- 依賴 --------------------
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

#-------------------- 來源檔 -------------------
add_executable(RED_Safe_server
    src/main.cpp
    src/api-server.cpp
    src/session.cpp
)

target_link_libraries(RED_Safe_server
    PRIVATE
      Boost::system
      Boost::thread
      OpenSSL::SSL
      OpenSSL::Crypto
)

#-------------------- 複製憑證 ------------------
configure_file(${CMAKE_SOURCE_DIR}/server.crt  server.crt  COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/server.key  server.key  COPYONLY)
