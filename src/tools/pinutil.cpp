/**
 * File:   $URL $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz Tools - PIN Entry Utilities
 *
 * Copyright (c) 2014, T Parys
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <fstream>
#include <string>
#include <topaz/exceptions.h>
#include "pinutil.h"
using namespace std;
using namespace topaz;

// Turn on character echo on terminal
void enable_terminal_echo()
{
  struct termios cur;
  
  // Get current terminal settings
  if (tcgetattr(STDIN_FILENO, &cur) != 0)
  {
    throw topaz_exception("Error getting terminal settings");
  }
  
  // Echo on
  cur.c_lflag |= ECHO;
  
  // Set current terminal settings
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) != 0)
  {
    throw topaz_exception("Error setting terminal settings");
  }
}

// Turn off character echo on terminal
void disable_terminal_echo()
{
  struct termios cur;
  
  // Get current terminal settings
  if (tcgetattr(STDIN_FILENO, &cur) != 0)
  {
    throw topaz_exception("Error getting terminal settings");
  }
  
  // Echo off
  cur.c_lflag &= ~ECHO;
  
  // Set current terminal settings
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) != 0)
  {
    throw topaz_exception("Error setting terminal settings");
  }
}

// Read a PIN from file
string pin_from_file(char const *path)
{
  // Open input file
  ifstream ifile(path);
  if (!ifile.good())
  {
    throw topaz_exception((string)"Cannot open file - " + path);
  }

  // Read data
  string pin;
  int in_char;
  while ((ifile.good()) && ((in_char = ifile.get()) != EOF))
  {
    pin += (char)in_char;
  }
  
  return pin;
}

// Read a PIN from console
string pin_from_console(char const *prompt)
{
  string pin;
  
  // Supress password echo to screen
  disable_terminal_echo();
  
  // Set up a simple prompt
  cout << "Please enter " << prompt << " PIN: " << flush;
  getline(cin, pin);
  cout << endl;
  
  // Restore typical behavior
  enable_terminal_echo();
  
  // Convert to atom
  return pin;
}
