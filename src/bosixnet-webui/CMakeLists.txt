project (bosixnet_webui)
cmake_minimum_required (VERSION 2.8)

include_directories(${CMAKE_BINARY_DIR})

add_executable (${PROJECT_NAME} main.cpp)

target_link_libraries (${PROJECT_NAME} fcgi)

execute_process (COMMAND gzip -9
                 INPUT_FILE ${PROJECT_NAME}.1
                 OUTPUT_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

install (TARGETS ${PROJECT_NAME}
         RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")
install (FILES bosixnet-webui
         DESTINATION "/etc/init.d/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES webui_launcher
         DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES bosixnet-webui.conf
         DESTINATION "/etc/bosixnet/")
install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.1.gz
         DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man1")
