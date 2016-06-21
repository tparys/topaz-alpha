###
# Topaz User Tools and Utilities
#

# TPer Admin SP tool
add_executable(tp_admin tp_admin.cpp)
target_link_libraries(tp_admin topaz)

# TPer Locking SP tool
add_executable(tp_lock spinner.cpp tp_lock.cpp)
target_link_libraries(tp_lock topaz)

# TPer Crypto Wipe
add_executable(tp_wipe tp_wipe.cpp)
target_link_libraries(tp_wipe topaz)

# TPer example unlock
add_executable(tp_unlock_simple tp_unlock_simple.cpp)
target_link_libraries(tp_unlock_simple topaz)

# MBR Shadow -> NBD bridge
add_executable(tp_mbr_bridge tp_mbr_bridge.cpp)
target_link_libraries(tp_mbr_bridge topaz pthread)
