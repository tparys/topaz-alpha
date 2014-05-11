#ifndef PINUTIL_H
#define PINUTIL_H

#include <topaz/atom.h>

// Turn on character echo on terminal
void enable_terminal_echo();

// Turn off character echo on terminal
void disable_terminal_echo();

// Read a PIN from file
topaz::atom pin_from_file(char const *path);

// Read a PIN from console
topaz::atom pin_from_console(char const *prompt);

#endif
