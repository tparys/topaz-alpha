###
# Topaz User Tools and Utilities
#

# TPer Admin SP tool
add_executable(tp_admin pinutil.cpp tp_admin.cpp)
target_link_libraries(tp_admin topaz)

# TPer Locking SP tool
add_executable(tp_lock pinutil.cpp spinner.cpp tp_lock.cpp)
target_link_libraries(tp_lock topaz)

# TPer Crypto Wipe
add_executable(tp_erase pinutil.cpp tp_erase.cpp)
target_link_libraries(tp_erase topaz)

# TPer example unlock
add_executable(tp_unlock_simple pinutil.cpp tp_unlock_simple.cpp)
target_link_libraries(tp_unlock_simple topaz)

