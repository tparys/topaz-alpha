#include <unistd.h>
#include <signal.h>
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

void ctl_c_handler(int sig);
void usage();
char const *lifecycle_to_string(uint64_t val);
void do_auth_login(drive &target, atom &prov_pin);

int main(int argc, char **argv)
{
  atom cur_pin, new_pin;
  char c;
  datum io;
  
  // Install handler for Ctl-C to restore terminal to sane state
  signal(SIGINT, ctl_c_handler);
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "p:P:n:N:v")) != -1)
  {
    switch (c)
    {
      case 'p':
	cur_pin = atom::new_bin(optarg);
	break;
	
      case 'P':
	cur_pin = pin_from_file(optarg);
	break;
	
      case 'n':
	new_pin = atom::new_bin(optarg);
	break;
	
      case 'N':
	new_pin = pin_from_file(optarg);
	break;
	
      case 'v':
        topaz_debug++;
        break;
        
      default:
	if ((optopt == 'p') || (optopt == 'P') || (optopt == 'n') || (optopt == 'N'))
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
  if ((argc - optind) < 2)
  {
    cerr << "Invalid number of arguments" << endl;
    usage();
    return -1;
  }
  
  // Open the device
  try
  {
    // Open the device, start as anonymous mode
    drive target(argv[optind]);
    datum io;
    target.login_anon(ADMIN_SP);
    
    // Determine our operation
    if (strcmp(argv[optind + 1], "status") == 0)
    {
      atom val;
      
      // Query lifecycle(6) status of Admin SP
      val = target.table_get(ADMIN_SP, 6);
      cout << "Admin SP   : " << lifecycle_to_string(val.get_uint()) << endl;
      
      // Query lifecycle(6) status of Locking SP
      val = target.table_get(LOCKING_SP, 6);
      cout << "Locking SP : " << lifecycle_to_string(val.get_uint()) << endl;
    }
    // Test admin credentials
    else if (strcmp(argv[optind + 1], "login") == 0)
    {
      // Authorized session needed
      do_auth_login(target, cur_pin);
      
      // Let user know status
      cout << "Login credentials OK" << endl;
    }
    // Change SID(Admin) PIN
    else if (strcmp(argv[optind + 1], "setpin") == 0)
    {
      // Authorized session needed
      do_auth_login(target, cur_pin);
      
      // Ensure new pin was provided
      if (new_pin.get_type() != atom::BYTES)
      {
	// No, query for one now
	new_pin = pin_from_console("new SID(admin)");
      }
      
      // Set PIN of SID (Drive Owner) in Admin SP
      target.table_set(C_PIN_SID, 3, new_pin);
    }
    // Activate Locking SP
    else if (strcmp(argv[optind + 1], "activate") == 0)
    {
      // Authorized session needed
      do_auth_login(target, cur_pin);
      
      // Locking_SP.Activate[]
      target.invoke(LOCKING_SP, ACTIVATE);
    }
    // Revert TPer (Admin & anything else)
    else if (strcmp(argv[optind + 1], "revert") == 0)
    {
      // Authorized session needed
      do_auth_login(target, cur_pin);
      
      // Admin_SP.Revert[]
      target.invoke(ADMIN_SP, REVERT);
    }
    else
    {
      // Nada
      cerr << "Unknown comand " << argv[optind + 1] << endl;
      usage();
      return -1;
    }
  }
  catch (topaz_exception &e)
  {
    cerr << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}

void ctl_c_handler(int sig)
{
  // Make sure this is on when program terminates
  enable_terminal_echo();
  exit(0);
}

void usage()
{
  cerr << endl
       << "Usage:" << endl
       << "  tp_admin [opts] <drive> status   - View current Admin SP status" << endl
       << "  tp_admin [opts] <drive> login    - Test SID(admin) login credentials" << endl
       << "  tp_admin [opts] <drive> setpin   - Set/Change SID(admin) PIN" << endl
       << "  tp_admin [opts] <drive> activate - Activate Locking SP" << endl
       << "  tp_admin [opts] <drive> revert   - Revert/Reset Admin SP (DATA LOSS!)" << endl
       << endl
       << "Options:" << endl
       << "  -p <pin>  - Provide current SID PIN" << endl
       << "  -P <file> - Read current PIN from file" << endl
       << "  -n <pin>  - Provide new SID PIN (setpin only)" << endl
       << "  -N <pin>  - Read new PIN from file (setpin only)" << endl
       << "  -v        - Increase debug verbosity" << endl;
}

char const *lifecycle_to_string(uint64_t val)
{
  // Table from Opal SSC 2.0 rev1, pg 96
  switch (val)
  {
    case 0:
      return "Inactive";
      
    case 1:
      return "Issued-Disabled";
      
    case 2:
      return "Issued-Frozen";
      
    case 3:
      return "Issued-Disabled-Frozen";
      
    case 4:
      return "Issued-Failed";
      
    case 8:
      return "Manufactured-Inactive";
      
    case 9:
      return "Manufactured";
      
    case 10:
      return "Manufactured-Disabled";
      
    case 11:
      return "Manufactured-Frozen";
      
    case 12:
      return "Manufactured-Disabled-Frozen";
      
    case 13:
      return "Manufactured-Failed";
      
    default:
      return "Reserved";
  }
}

// Perform authorized login to Admin SP
void do_auth_login(drive &target, atom &prov_pin)
{
  // Was a PIN provided?
  if (prov_pin.get_type() == atom::BYTES)
  {
    // Try and use that
    target.login(ADMIN_SP, SID, prov_pin.get_bytes());
    return;
  }
  
  // Failing that, try the default PIN
  atom pin = target.default_pin();
  try
  {
    target.login(ADMIN_SP, SID, pin.get_bytes());
    return;
  }
  catch (topaz_exception &e)
  {
    // Didn't work
  }
  
  // Last effort - prompt user for PIN to use ...
  pin = pin_from_console("SID(admin)");
  target.login(ADMIN_SP, SID, pin.get_bytes());
}
