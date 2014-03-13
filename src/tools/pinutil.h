#ifndef PINUTIL_H
#define PINUTIL_H

#include <topaz/atom.h>

// Read a PIN from file
topaz::atom pin_from_file(char const *path);

// Read a PIN from console
topaz::atom pin_from_console(char const *prompt);

#endif
