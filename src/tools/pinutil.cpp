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
atom pin_from_file(char const *path)
{
  // Open input file
  ifstream ifile(path);
  if (!ifile.good())
  {
    throw topaz_exception((string)"Cannot open file - " + path);
  }

  // Read data
  byte_vector bytes;
  int in_char;
  while ((in_char = ifile.good()) != EOF)
  {
    bytes.push_back(in_char);
  }
  
  return atom::new_bin(bytes);
}

// Read a PIN from console
atom pin_from_console(char const *prompt)
{
  string pin_str;
  
  // Supress password echo to screen
  disable_terminal_echo();
  
  // Set up a simple prompt
  cout << "Please enter " << prompt << " PIN: " << flush;
  getline(cin, pin_str);
  cout << endl;
  
  // Restore typical behavior
  enable_terminal_echo();
  
  // Convert to atom
  return atom::new_bin(pin_str.c_str());
}
