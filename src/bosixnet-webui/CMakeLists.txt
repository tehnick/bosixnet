project (bosixnet_webui)
cmake_minimum_required (VERSION 2.8)

include_directories(${CMAKE_BINARY_DIR})

add_executable (${PROJECT_NAME} bosixnet-webui.cpp)

target_link_libraries (${PROJECT_NAME} fcgi)

execute_process (COMMAND gzip -9
                 INPUT_FILE ${PROJECT_NAME}.1
                 OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

install (TARGETS ${PROJECT_NAME}
         RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")
install (FILES webui_launcher
         DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES bosixnet-webui.conf
         DESTINATION "/etc/bosixnet/")
install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
         DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1")

if (WITH_SYSVINIT_SUPPORT)
    install (FILES bosixnet-webui
             DESTINATION "${SYSVINIT_SCRIPTS_DIR}"
             PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif (WITH_SYSVINIT_SUPPORT)

if (WITH_SYSTEMD_SUPPORT)
    install (FILES bosixnet-webui.service
             DESTINATION "${SYSTEMD_UNIT_FILES_DIR}"
             PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
endif (WITH_SYSTEMD_SUPPORT)
