project (bosixnet-daemon)
cmake_minimum_required (VERSION 2.8)

install (FILES bosixnet_daemon
         DESTINATION "/usr/bin/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES bosixnet-daemon
         DESTINATION "/etc/init.d/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES update_address update_hosts
         DESTINATION "/usr/lib/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install (FILES bosixnet-daemon.conf
         DESTINATION "/etc/bosixnet/"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

