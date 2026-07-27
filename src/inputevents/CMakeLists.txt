
configure_file(unixkey-addon.conf.in unixkey-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/unixkey-addon.conf" RENAME unixkey.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

install(FILES "unixkey.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
