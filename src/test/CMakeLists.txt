###
# Project Stuff
#

add_executable(test test.cpp)
target_link_libraries(test topaz)

add_executable(test-atom test-atom.cpp)
target_link_libraries(test-atom topaz)

add_executable(test-datum test-datum.cpp)
target_link_libraries(test-datum topaz)
