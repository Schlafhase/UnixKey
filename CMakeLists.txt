cmake_minimum_required(VERSION 3.21)
project(fcitx5-unixkey)

find_package(Fcitx5Core REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc io)
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")

add_library(fcitx5_index_dummy OBJECT EXCLUDE_FROM_ALL fcitx5-index.cpp)
target_link_libraries(fcitx5_index_dummy PRIVATE Fcitx5::Core Fcitx5::Utils Fcitx5::Config)

add_subdirectory(src)
