# Check if IPO is supported
include(CheckIPOSupported)
check_ipo_supported(RESULT HAVE_IPO)

# Enable IPO in non-debug build
macro(target_enable_ipo NAME)
  if(NOT CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG" AND HAVE_IPO)
    set_property(TARGET ${NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    message (STATUS "Enabled IPO for target: ${NAME}")
  endif()
endmacro()

macro(target_add_lib NAME)
  file(GLOB_RECURSE FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" "*.hpp")
  add_library(${NAME} STATIC ${FILES} ${FBS_FILES})
  target_include_directories(${NAME}
      PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
  target_link_libraries(${NAME} ${ARGN} "")
endmacro()

macro(target_add_shared_lib NAME)
  file(GLOB_RECURSE FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" "*.hpp")
  add_library(${NAME} SHARED ${FILES} ${FBS_FILES})
  target_include_directories(${NAME}
      PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
  target_link_libraries(${NAME} ${ARGN} "")
  target_enable_ipo(${NAME})
endmacro()

macro(target_add_bin NAME MAIN_FILE)
  add_executable(${NAME} ${MAIN_FILE})
  target_link_libraries(${NAME} ${ARGN} "")
  target_include_directories(${NAME}
      PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/src/lib/api
      ${PROJECT_BINARY_DIR}/src
      ${PROJECT_BINARY_DIR}
  )
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  target_enable_ipo(${NAME})
endmacro()