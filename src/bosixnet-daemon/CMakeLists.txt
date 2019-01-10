project (bosixnet_daemon)
cmake_minimum_required (VERSION 2.8)

execute_process (COMMAND gzip -9
                 INPUT_FILE ${PROJECT_NAME}.1
                 OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

install (FILES ${PROJECT_NAME}
         DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES daemon_launcher update_address update_hosts
         DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES bosixnet-daemon.conf
         DESTINATION "/etc/bosixnet/")
install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
         DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1")

if (WITH_SYSTEMD_SUPPORT)
    install (FILES bosixnet-daemon.service
             DESTINATION "${SYSTEMD_UNIT_FILES_DIR}"
             PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif (WITH_SYSTEMD_SUPPORT)
