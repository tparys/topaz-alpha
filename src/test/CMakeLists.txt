###
# Project Stuff
#

add_executable(test test.cpp)
target_link_libraries(test topaz)

add_executable(test-syntax test-syntax.cpp)
target_link_libraries(test-syntax topaz)
