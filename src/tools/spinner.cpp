#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "spinner.h"

// Spinner characters
char const *spin_chars = "|/-\\";

// Constructor
spinner::spinner(int max)
  : cur(0), max(max), width(80), old_pos(0)
{
  int i;
  
  // Initial layout
  for (i = 0; i < width - 1; i++)
  {
    putchar(' ');
  }
  fputs("|\r| ", stdout);
  fflush(stdout);
  
  // Account for left/right edges
  width -= 2;
}

// Destructor
spinner::~spinner()
{
  putchar('\n');
  fflush(stdout);
}

// Mutator
void spinner::tick(int count)
{
  int new_pos;
  
  // Increment
  cur += count;
  
  // Calculate new position on progress
  new_pos = round((double)cur * width / max);
  
  // Move back one character
  putchar('\b');
  
  // Place progress meter, if appropriate
  for (int i = 0; i < (new_pos - old_pos); i++)
  {
    putchar('=');
  }
  old_pos = new_pos;
  
  // Update spinner
  putchar(spin_chars[cur % 4]);
  
  // Flush
  fflush(stdout);
}


