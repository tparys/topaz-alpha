/**
 * Topaz - PIN Entry
 *
 * Functions and definitions for securely entering PINs and passwords.
 *
 * Copyright (c) 2016, T Parys
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
#include <iostream>
#include <fstream>
#include <signal.h>
#include <termios.h>
#include <topaz/pin_entry.h>
#include <topaz/exceptions.h>
using namespace std;
using namespace topaz;

/**
 * Set terminal echo
 *
 * \param echo Enable echo on true, false otherwise
 */
void topaz::set_terminal_echo(bool echo)
{
  struct termios cur;
  
  // Get current terminal settings
  if (tcgetattr(STDIN_FILENO, &cur) != 0)
  {
    throw topaz_exception("Error getting terminal settings");
  }
  
  // Echo on or off
  if (echo)
  {
    cur.c_lflag |= ECHO;
  }
  else
  {
    cur.c_lflag &= ~ECHO;
  }
  
  // Set current terminal settings
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur) != 0)
  {
    throw topaz_exception("Error setting terminal settings");
  }
}

/**
 * PIN Entry Signal Handler
 */
void topaz::pin_signal_handler(int signum, siginfo_t *info, void *ctx)
{
  // Restore console echo
  set_terminal_echo(true);
}

/**
 * Read a PIN from file
 *
 * \param path File path to read
 * \return String containing file contents
 */
string topaz::pin_from_file(char const *path)
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

/**
 * Read a PIN from console
 *
 * \param prompt Name of PIN to request
 * \return String containing entered PIN
 */
string topaz::pin_from_console(char const *prompt)
{
  string pin;

  // Set up handler if someone hits Ctrl-C (INT) or sends a TERM
  struct sigaction new_action = {0}, old_action[2];
  new_action.sa_sigaction = pin_signal_handler;
  new_action.sa_flags = SA_SIGINFO;
  if ((sigaction(SIGINT, &new_action, old_action + 0) < 0) ||
      (sigaction(SIGTERM, &new_action, old_action + 1) < 0))
  {
    throw topaz_exception("Cannot set PIN signal handler");
  }

  // Disable console echo
  set_terminal_echo(false);

  // Set up a simple prompt
  cout << "Please enter " << prompt << " PIN: " << flush;
  getline(cin, pin);
  cout << endl;

  // Restore console echo
  set_terminal_echo(true);

  // Restore signal handlers
  if ((sigaction(SIGINT, old_action + 0, NULL) < 0) ||
      (sigaction(SIGTERM, old_action + 1, NULL) < 0))
  {
    throw topaz_exception("Cannot set PIN signal handler");
  }

  return pin;
}

/**
 * Read a PIN from console with confirmation
 *
 * \param prompt Name of PIN to request
 * \return String containing entered PIN
 */
string topaz::pin_from_console_check(char const *prompt)
{
  string pin1, pin2;

  pin1 = pin_from_console(prompt);
  cout << "One more time to confirm ..." << endl;
  pin2 = pin_from_console(prompt);

  if (pin1 != pin2)
  {
    throw topaz_exception("Entered PINs do not match");
  }

  return pin1;
}

