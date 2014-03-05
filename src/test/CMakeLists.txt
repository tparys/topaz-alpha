###
# Project Stuff
#

add_executable(test test.cpp)
target_link_libraries(test topaz)

add_executable(test-atom test-atom.cpp)
target_link_libraries(test-atom topaz)

add_executable(test-datum test-datum.cpp)
target_link_libraries(test-datum topaz)

add_executable(adminsp-set-pin adminsp-set-pin.cpp)
target_link_libraries(adminsp-set-pin topaz)

add_executable(adminsp-revert adminsp-revert.cpp)
target_link_libraries(adminsp-revert topaz)
