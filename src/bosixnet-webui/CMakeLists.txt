project (bosixnet-webui)
cmake_minimum_required (VERSION 2.8)

include_directories(${CMAKE_BINARY_DIR})

add_executable (bosixnet_webui main.cpp)

target_link_libraries (bosixnet_webui fcgi)

install (TARGETS bosixnet_webui RUNTIME DESTINATION bin)
install (FILES bosixnet-webui
         DESTINATION "/etc/init.d/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES webui_launcher
         DESTINATION "/usr/lib/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES bosixnet-webui.conf
         DESTINATION "/etc/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

