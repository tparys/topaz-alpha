###
# Project Stuff
#

set(TOPAZ_SRCS
  atom.cpp
  datum.cpp
  debug.cpp
  drive.cpp
  encodable.cpp
  rawdrive.cpp
  pin_entry.cpp
)

add_library(topaz ${TOPAZ_SRCS})
