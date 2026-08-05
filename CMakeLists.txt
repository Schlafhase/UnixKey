cmake_minimum_required(VERSION 3.21)
project(fcitx5-unixkey)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

find_package(Fcitx5Core REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc io)
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

add_library(fcitx5_index_dummy OBJECT EXCLUDE_FROM_ALL fcitx5-index.cpp)
target_link_libraries(fcitx5_index_dummy PRIVATE Fcitx5::Core Fcitx5::Utils Fcitx5::Config)

add_subdirectory(src)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_program(IWYU_TOOL_PY iwyu_tool.py HINTS ${CMAKE_SOURCE_DIR}/tools)
  find_program(FIX_INCLUDES_PY fix_includes.py HINTS ${CMAKE_SOURCE_DIR}/tools)
  if(IWYU_TOOL_PY AND FIX_INCLUDES_PY)
    add_custom_target(fix-includes
      COMMAND ${IWYU_TOOL_PY} -p ${CMAKE_BINARY_DIR} | ${FIX_INCLUDES_PY} --nosafe_headers --nocomments
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      VERBATIM USES_TERMINAL
    )
  endif()
endif()
