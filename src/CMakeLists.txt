
add_library(unixkey SHARED unixkey.cpp)
target_link_libraries(unixkey PRIVATE Fcitx5::Core)
target_link_libraries(unixkey PRIVATE ICU::uc ICU::io)
set_target_properties(unixkey PROPERTIES PREFIX "")
install(TARGETS unixkey DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")


configure_file(unixkey-addon.conf.in unixkey-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/unixkey-addon.conf" RENAME unixkey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

install(FILES "unixkey.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
