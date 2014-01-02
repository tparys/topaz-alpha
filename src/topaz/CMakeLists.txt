###
# Project Stuff
#

set(TOPAZ_SRCS
  atom.cpp
  debug.cpp
  drive.cpp
  rawdrive.cpp
  serializable.cpp
)

add_library(topaz ${TOPAZ_SRCS})
