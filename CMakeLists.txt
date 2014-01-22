cmake_minimum_required(VERSION 2.4)

###
# Compiler flags
#

# GCC
if (CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_C_FLAGS "-O2 -Wall -fmax-errors=5")
  #set (CMAKE_C_FLAGS "-O2 -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
endif (CMAKE_COMPILER_IS_GNUCC)

###
# Project Stuff
#

# Put all binaries in one place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${Project_SOURCE_DIR}/build")

# Where includes get sourced
include_directories(src)

# Start building
subdirs(src)
