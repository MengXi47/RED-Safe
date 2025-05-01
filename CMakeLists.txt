cmake_minimum_required(VERSION 3.10)
project(RED_Safe VERSION 1.0
                DESCRIPTION "RED-Safe"
                LANGUAGES CXX)

# 預設使用 C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 方便用者一行指令就切換 Debug / Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build (Debug or Release)" FORCE)
endif()

# 將 api-server 當作子專案
add_subdirectory(api-server)
