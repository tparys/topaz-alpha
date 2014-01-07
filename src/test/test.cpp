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
    topaz::datum call, ret;
    
    // C_PIN_MSID.Get[StartCol = C_PIN, EndCol = C_PIN]
    call = topaz::datum();
    call.object_uid()  = topaz::OBJ_C_PIN_MSID;
    call.method_uid()  = topaz::MTH_GET;
    call[0][0].name()  = topaz::atom((uint64_t)3); // Starting Table Column
    call[0][0].value() = topaz::atom((uint64_t)3); // C_PIN
    call[0][1].name()  = topaz::atom((uint64_t)4); // Ending Tabling Column
    call[0][1].value() = topaz::atom((uint64_t)3); // C_PIN
    drive.sendrecv(call, ret);
    
    // Default pin here
    topaz::atom pin = ret[0][0].value();
    printf("Default PIN: ");
    pin.print();
    printf("\n");
    
    // AdminSP.Revert[]
    call = topaz::datum();
    call.object_uid() = topaz::OBJ_ADMIN_SP;
    call.method_uid() = topaz::MTH_REVERT;
    drive.sendrecv(call, call);
  }
  catch (topaz::topaz_exception &e)
  {
    cout << "Exception raised: " << e.what() << endl;
  }
  
  return 0;
}
