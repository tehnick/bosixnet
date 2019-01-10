project (bosixnet)
cmake_minimum_required (VERSION 2.8)

set (PROJECT_NAME "BOSixNet")
set (VERSION "2.0")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

option (WITH_SYSTEMD_SUPPORT "Install systemd unit configuration files" ON)

if (WITH_SYSTEMD_SUPPORT)
    if (NOT SYSTEMD_UNIT_FILES_DIR)
        set (SYSTEMD_UNIT_FILES_DIR "/lib/systemd/system/")
    endif (NOT SYSTEMD_UNIT_FILES_DIR)
    message (STATUS "Systemd unit configuration files will be installed to: ${SYSTEMD_UNIT_FILES_DIR}")
endif (WITH_SYSTEMD_SUPPORT)

add_subdirectory (src/bosixnet-daemon)
add_subdirectory (src/bosixnet-webui)

configure_file (${PROJECT_SOURCE_DIR}/Version.h.in ${PROJECT_BINARY_DIR}/Version.h)

add_custom_target (uninstall
    COMMAND xargs -a "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt" rm
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/src/bosixnet-daemon/bosixnet-daemon remove
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/src/bosixnet-webui/bosixnet-webui remove)

