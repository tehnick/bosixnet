project (bosixnet)
cmake_minimum_required (VERSION 2.8)

set (PROJECT_NAME "BOSixNet")
set (VERSION "1.1")

add_subdirectory (src/bosixnet-daemon)
add_subdirectory (src/bosixnet-webui)

configure_file (${PROJECT_SOURCE_DIR}/Version.h.in ${PROJECT_BINARY_DIR}/Version.h)

add_custom_target (uninstall
    COMMAND xargs -a "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" rm
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/src/bosixnet-daemon/bosixnet-daemon remove
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/src/bosixnet-webui/bosixnet-webui remove)

