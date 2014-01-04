#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>
#include <topaz/datum.h>
using namespace std;

int main(int argc, char **argv)
{
  char c;

  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "v")) != -1)
  {
    switch (c)
    {
      case 'v':
        topaz_debug++;
        break;
        
      default:
        cerr << "Invalid command line option " << c << endl;
    }
  }
  
  // Check remaining arguments
  if ((argc - optind) <= 0)
  {
    cerr << "Usage: test /dev/sdX" << endl;
    return -1;
  }
 
  // Open the device
  try
  {
    topaz::drive drive(argv[optind]);

    // Try to retrieve the device's default PIN
    topaz::byte_vector pin = drive.default_pin();
    bool printable = true;
    printf("Got a PIN (%lu bytes):", pin.size());
    for (size_t i = 0; i < pin.size(); i++)
    {
      if ((i % 16) == 0)
      {
	printf("\n");
      }
      printf(" %02X", pin[i]);
      
      if (!isprint(pin[i]))
      {
	printable = false;
      }
    }
    printf("\n");
    
    // Is it printable?
    if (printable)
    {
      printf("ASCII: ");
      for (size_t i = 0; i < pin.size(); i++)
      {
	printf("%c", pin[i]);
      }
      printf("\n");
    }
    
  }
  catch (topaz::exception &e)
  {
    cout << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}
