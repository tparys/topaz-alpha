/**
 * Topaz Tools - Cryptographic Wipe
 *
 * Demonstration application showcasing cryptographic wipe within a TCG Opal
 * compliant Self-Encrypting Hard Drive (SED).
 *
 * Copyright (c) 2014, T Parys
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
#include <topaz/pin_entry.h>
using namespace std;
using namespace topaz;

void usage();

int main(int argc, char **argv)
{
  char c;
  uint64_t uid = 0;
  string pin;
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "vs:p:")) != -1)
  {
    switch (c)
    {
      
      case 's':
        // SID credentials
        uid = SID;
        pin = optarg;
        break;
 
      case 'p':
        // PSID credentials
        uid = PSID;
        pin = optarg;
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
      target.login(ADMIN_SP, uid, pin);
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
      target.login(ADMIN_SP, uid, pin);
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
       << "  tp_wipe [opts] <drive> - Cryptographic wipe of drive" << endl
       << endl
       << "Options:" << endl
       << "  -v        - Increase debug verbosity" << endl
       << "  -s <pin>  - Use SID credentials for drive wipe" << endl
       << "  -p <pin>  - Use PSID credentials for drive wipe" << endl;
}
