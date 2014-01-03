#include <unistd.h>
#include <stdio.h>
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
  }
  catch (topaz::exception &e)
  {
    cout << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}
