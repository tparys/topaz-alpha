/**
 * Topaz Tools - Simple Unlock Program
 *
 * Demonstration application to showcase how to programatically unlock a TCG
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
#include <signal.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <iomanip>
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>
#include <topaz/uid.h>
#include <topaz/pin_entry.h>
using namespace std;
using namespace topaz;

void usage();
uint64_t get_uid(char const *user_str);
bool unlock_target(char const *path, uint64_t user_uid, string pin,
                   uint64_t range_count = 1);

int main(int argc, char **argv)
{
  string pin;
  bool pin_valid = false;
  uint64_t user_uid = ADMIN_BASE + 1;
  uint64_t lba_count = 1;
  char c;
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "u:p:r:")) != -1)
  {
    switch (c)
    {
      case 'u':
        user_uid = get_uid(optarg);
        break;
 
      case 'p':
        pin = optarg;
        pin_valid = true;
        break;
 
      case 'r':
        lba_count = atoi(optarg);
        break;
 
      default:
        if ((optopt == 'u') || (optopt == 'p') || (optopt == 'r'))
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
  drive target(argv[optind]);
  
  // Loop until we unlock the drive
  while (1)
  {
    // Do we have credentials?
    if (!pin_valid)
    {
      // No, query for them now ...
      pin = pin_from_console("user");
    }
    
    // Attempt drive unlock
    if (unlock_target(argv[optind], user_uid, pin, lba_count))
    {
      // Succeeded
      break;
    }
    else
    {
      // Failed, clear credentials and try again
      pin_valid = false;
    }
  }
  
  // If additional drives are specified, try to unlock those too
  while (++optind < argc)
  {
    unlock_target(argv[optind], user_uid, pin, lba_count);
  }
  
  return 0;
}

void usage()
{
  cerr << endl
       << "Usage:" << endl
       << "  tp_unlock_simple [opts] <drive> - Simple unlock of TCG Opal drive" << endl
       << endl
       << "Options:" << endl
       << "  -p <pin>  - Provide PIN credentials" << endl
       << "  -u <user> - Specify user (default admin1)" << endl
       << "  -r <num>  - Unlock first <num> LBA ranges (default 1)" << endl;
}

uint64_t get_uid(char const *user_str)
{
  uint64_t base = 0;
  unsigned int num = 0;
  
  // Users come in two patterns:
  if (sscanf(user_str, "admin%u", &num) == 1)
  {
    base = ADMIN_BASE;
  }
  else if (sscanf(user_str, "user%u", &num) == 1)
  {
    base = USER_BASE;
  }
  else
  {
    // Illegal
    throw topaz_exception("Illegal Locking SP user");
  }
  
  return base + num;
}

bool unlock_target(char const *path, uint64_t user_uid, string pin,
                   uint64_t range_count)
{
  try
  {
    // Subject target
    drive target(path);
    
    // Login with specified credentials
    target.login(LOCKING_SP, user_uid, pin);
    
    // MBR Shadow isn't needed when unlocked, (1 -> hide it)
    target.table_set(MBR_CONTROL, 2, 1);
    
    // Clear "Read Lock"(7) on global range (0 -> turn it off)
    target.table_set(LBA_RANGE_GLOBAL, 7, 0);
      
    // Clear "Write Lock"(8) on global range (0 -> turn it off)
    target.table_set(LBA_RANGE_GLOBAL, 8, 0);
    
    // If more than one LBA range specified, unlock the next few as well
    for (uint64_t count = 1; count < range_count; count++)
    {
      // UID of next LBA range
      uint64_t lba_uid = LBA_RANGE_BASE + count;
      
      // Clear "Read Lock"(7) on this LBA range (0 -> turn it off)
      target.table_set(lba_uid, 7, 0);
      
      // Clear "Write Lock"(8) on this LBA range (0 -> turn it off)
      target.table_set(lba_uid, 8, 0);
    }
    
    // Succeeded
    return true;
  }
  catch (topaz_exception &e)
  {
    // Failed
    return false;
  }
}
