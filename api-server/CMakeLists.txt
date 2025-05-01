# api-server/CMakeLists.txt

# 建立一個可執行檔 target
add_executable(RED_safe_server
    main.cpp
)

# （可選）設定 include 目錄
# target_include_directories(red_safe_server PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 不同平台或編譯器專屬設定
if(APPLE)
  message(STATUS "Building on macOS")
  # 例如 Link 到 CoreFoundation（若需要）
  # target_link_libraries(red_safe_server PRIVATE "-framework CoreFoundation")
elseif(WIN32)
  message(STATUS "Building on Windows")
  # Windows-specific flags or libs
elseif(UNIX)
  message(STATUS "Building on Linux/Unix")
  # Linux-specific flags or libs
endif()

# （可選）啟用更多警告
target_compile_options(red_safe_server PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
