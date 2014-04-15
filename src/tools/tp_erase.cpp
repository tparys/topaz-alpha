#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>
#include <topaz/datum.h>
#include <topaz/uid.h>
#include "pinutil.h"
using namespace std;
using namespace topaz;

void usage();

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
	break;
    }
  }
  
  // Check remaining arguments
  if ((argc - optind) < 2)
  {
    cerr << "Invalid number of arguments" << endl;
    usage();
    return -1;
  }
  
  // Open the device
  try
  {
    // Open device
    drive target(argv[optind]);
    
    // PSID PIN
    atom psid_pin = atom::new_bin(argv[optind + 1]);
    
    // Log in as PSID authority
    target.login(ADMIN_SP, PSID, psid_pin.get_bytes());
    
    // Query Manufactured default SID credentials (will need these later)
    atom msid_pin = target.default_pin();
    
    // 1st Reset - Revert TPer to factory state
    // (NOTE - if Locking SP is inactive, this will not wipe drive)
    target.invoke(ADMIN_SP, REVERT);
    
    // Login to SID with default credentials
    target.login(ADMIN_SP, SID, msid_pin.get_bytes());
    
    // Turn Locking SP on for next reset
    target.invoke(LOCKING_SP, ACTIVATE);
    
    // 2nd Reset - Revert TPer to factory state
    // (NOTE - Locking SP is now active, goodbye everything ...)
    target.invoke(ADMIN_SP, REVERT);
  }
  catch (topaz_exception &e)
  {
    cerr << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}

void usage()
{
  cerr << endl
       << "Usage:" << endl
       << "  tp_erase [opts] <drive> <psid> - Cryptographic wipe to factory state" << endl
       << endl
       << "Options:" << endl
       << "  -v        - Increase debug verbosity" << endl;
}
