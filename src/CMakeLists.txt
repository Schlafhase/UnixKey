add_library(unixkey SHARED inputevents/unixkey.cpp inputevents/unixkeystate.cpp substitution/matcher.cpp substitution/config.cpp substitution/logger.h)
target_link_libraries(unixkey PRIVATE Fcitx5::Core)
target_link_libraries(unixkey PRIVATE ICU::uc ICU::io)
set_target_properties(unixkey PROPERTIES PREFIX "")
install(TARGETS unixkey DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

add_subdirectory(inputevents)
add_subdirectory(test)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(unixkey PRIVATE ENABLE_DEBUG_LOGS)
  target_compile_definitions(UnixKeyTest PRIVATE ENABLE_DEBUG_LOGS)
endif()
