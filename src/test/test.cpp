#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>
#include <topaz/datum.h>
#include <topaz/uid.h>
using namespace std;
using namespace topaz;

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
    datum io;
    drive target(argv[optind]);
    
    target.login_anon(ADMIN_SP);
    
    target.invoke(ADMIN_SP, REVERT);
  }
  catch (topaz_exception &e)
  {
    cout << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}
