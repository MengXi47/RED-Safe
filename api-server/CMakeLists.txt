# api-server/CMakeLists.txt

add_executable(RED_Safe_server
    main.cpp
)

# 不同平台或編譯器專屬設定
if(APPLE)
  message(STATUS "Building on macOS")
elseif(WIN32)
  message(STATUS "Building on Windows")
elseif(UNIX)
  message(STATUS "Building on Linux/Unix")
endif()

