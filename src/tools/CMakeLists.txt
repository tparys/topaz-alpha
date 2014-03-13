###
# Topaz User Tools and Utilities
#

# TPer Admin SP tool
add_executable(tp_admin pinutil.cpp tp_admin.cpp)
target_link_libraries(tp_admin topaz)

# TPer Locking SP tool
add_executable(tp_lock pinutil.cpp tp_lock.cpp)
target_link_libraries(tp_lock topaz)

