#include <iostream>
#include <fstream>
#include <string>
#include <topaz/exceptions.h>
#include "pinutil.h"
using namespace std;
using namespace topaz;

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
  
  // Set up a simple prompt (TBD - improve later)
  cout << "Please enter " << prompt << " PIN: " << flush;
  getline(cin, pin_str);
  
  // Convert to atom
  return atom::new_bin(pin_str.c_str());
}
