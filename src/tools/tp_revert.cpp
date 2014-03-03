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
    cerr << "Usage: tp_revert /dev/sdX" << endl;
    return -1;
  }
 
  // Open the device
  try
  {
    datum io;
    drive target(argv[optind]);
    
    target.login_anon(ADMIN_SP);
    
    atom mpin = target.default_pin();
    atom tpin = atom::new_bin("hello world");
    
    if (1)
    {
      target.login(ADMIN_SP, SID, mpin.get_bytes());
    }
    else
    {
      target.login_anon(ADMIN_SP);
    }
    
    // AdminSP.RevertSP[]
    io.object_uid() = LOCKING_SP;
    io.method_uid() = REVERT_SP;
    target.sendrecv(io);
  }
  catch (topaz_exception &e)
  {
    cout << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}
