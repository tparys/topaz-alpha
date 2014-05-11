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
  uint64_t uid = 0;
  atom pin;
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "vs:p:")) != -1)
  {
    switch (c)
    {
      
      case 's':
	// SID credentials
	uid = SID;
	pin = atom::new_bin(optarg);
	break;
	
      case 'p':
	// PSID credentials
	uid = PSID;
	pin = atom::new_bin(optarg);
	break;
	
      case 'v':
	topaz_debug++;
        break;
	
      default:
	if ((optopt == 's') || (optopt == 'p'))
	{
	  cerr << "Option -" << optopt << " requires an argument." << endl;
	}
	else
	{
	  cerr << "Invalid command line option " << c << endl;
	}
	break;
    }
  }
  
  // Check remaining arguments
  if ((argc - optind) < 1)
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
    
    // If no credentials specified, assume
    // Manufactured default SID (MSID) PIN
    if (uid == 0)
    {
      target.login_anon(ADMIN_SP);
      uid = SID;
      pin = target.default_pin();
    }
    
    // Attempt authenticated login
    try
    {
      target.login(ADMIN_SP, uid, pin.get_bytes());
    }
    catch (exception &e)
    {
      // Failed login
      cerr << "Invalid credentials presented to drive" << endl
	   << "Must present valid SID or PSID pin" << endl;
      return 1;
    }
    
    // The following code attempts to perform a drive wipe, by
    // invoking Admin_SP.Revert[] while the Locking_SP is activated,
    // subject to the following constraints:
    //
    // PSID:
    //   - Cannot activate Locking_SP
    //   - Can recover SID credentials to MSID
    //
    // SID:
    //   - Can activate Locking_SP
    //   - May have been deactivated, or set to unknown PIN
    
    // Check if Locking_SP is in any state other than Manufactured-Inactive(8),
    // if it is, then a Revert[] from either SID will complete operation
    bool lock_active = target.table_get(LOCKING_SP, 6).get_uint() != 8;
    
    ////
    // PSID specific operations
    //
    
    if (uid == PSID)
    {
      // Call revert
      target.admin_sp_revert();
      
      // If Locking_SP was active, it's one and done
      if (lock_active)
      {
	return 0;
      }
      
      // If not, SID credentials have been reset to
      // MSID defaults, so get those and continue ...
      target.login_anon(ADMIN_SP);
      uid = SID;
      pin = target.default_pin();
      
      // Authenticated login
      target.login(ADMIN_SP, uid, pin.get_bytes());
    }
    
    ////
    // SID specific operations
    //
    
    // If Locking_SP is not active, turn it on now
    if (!lock_active)
    {
      target.invoke(LOCKING_SP, ACTIVATE);
    }
    
    // Final revert
    target.admin_sp_revert();
    
    // Drive scrubbed, all data gone ...
    
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
       << "  tp_erase [opts] <drive> - Cryptographic wipe of drive" << endl
       << endl
       << "Options:" << endl
       << "  -v        - Increase debug verbosity" << endl
       << "  -s <pin>  - Use SID credentials for drive wipe" << endl
       << "  -p <pin>  - Use PSID credentials for drive wipe" << endl;
}
