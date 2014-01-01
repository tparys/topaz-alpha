###
# Project Stuff
#

set(TOPAZ_SRCS
  debug.cpp
  drive.cpp
  rawdrive.cpp
  syntax.cpp
)

add_library(topaz ${TOPAZ_SRCS})
