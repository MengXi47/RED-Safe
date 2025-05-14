cmake_minimum_required(VERSION 3.15)

project(RED_Safe_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-------------------- 來源檔 -------------------
add_executable(RED_Safe_apiserver
    main.cpp
    src/api-server.cpp
    src/session.cpp
    src/controller.cpp
    src/service/registration_service.cpp
    src/model/sql_model.cpp
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
elseif(UNIX)
  message(STATUS "Building on Linux/Unix")
  target_compile_definitions(RED_Safe_apiserver PRIVATE PLATFORM_UNIX)
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

#-------------------- 依賴 --------------------
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(nlohmann_json REQUIRED)
find_package(libpqxx CONFIG QUIET)

if(TARGET libpqxx::pqxx)                       # 成功找到
    set(PQXX_TARGET libpqxx::pqxx)
else()                                         # 回退到 pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PQXX REQUIRED libpqxx)
    add_library(pqxx INTERFACE)
    target_include_directories(pqxx INTERFACE ${PQXX_INCLUDE_DIRS})
    target_link_libraries(pqxx INTERFACE ${PQXX_LINK_LIBRARIES})
    set(PQXX_TARGET pqxx)
endif()


target_include_directories(RED_Safe_apiserver
    PRIVATE
      ${Boost_INCLUDE_DIRS}
      ${nlohmann_json_INCLUDE_DIRS}
)
target_link_libraries(RED_Safe_apiserver
    PRIVATE
      ${Boost_LIBRARIES}
      ${nlohmann_json_LIBS}
      ${PQXX_TARGET}
)
