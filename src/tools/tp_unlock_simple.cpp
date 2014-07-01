/**
 * File:   $URL $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz Tools - Simple Unlock Program
 *
 * Demonstration application to showcase how to programatically unlock 
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
#include <topaz/datum.h>
#include <topaz/uid.h>
#include "pinutil.h"
using namespace std;
using namespace topaz;

void ctl_c_handler(int sig);
void usage();
uint64_t get_uid(char const *user_str);

int main(int argc, char **argv)
{
  atom user_pin;
  uint64_t user_uid = ADMIN_BASE + 1;
  char c;
  
  // Install handler for Ctl-C to restore terminal to sane state
  signal(SIGINT, ctl_c_handler);
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "u:p:")) != -1)
  {
    switch (c)
    {
      case 'u':
	user_uid = get_uid(optarg);
	break;
	
      case 'p':
	user_pin = atom::new_bin(optarg);
	break;
	
      default:
	if (optopt == 'p')
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
    if (user_pin.get_type() != atom::BYTES)
    {
      // No, query for them now ...
      user_pin = pin_from_console("user");
    }
    
    // Attempt drive unlock
    try
    {
      // Login with specified credentials
      target.login(LOCKING_SP, user_uid, user_pin.get_bytes());
      
      // We are "Done"(2) with the MBR shadow (1 -> hide it)
      target.table_set(MBR_CONTROL, 2, atom::new_uint(1));
      
      // Clear "Read Lock"(7) on global range (0 -> turn it off)
      target.table_set(LBA_RANGE_GLOBAL, 7, atom::new_uint(0));
      
      // Clear "Write Lock"(8) on global range (0 -> turn it off)
      target.table_set(LBA_RANGE_GLOBAL, 8, atom::new_uint(0));
      
      // All done
      break;
    }
    catch (topaz_exception &e)
    {
      // Failed, clear credentials and try again
      user_pin = atom();
    }
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
       << "  tp_unlock_simple [opts] <drive> - Simple unlock of TCG Opal drive" << endl
       << endl
       << "Options:" << endl
       << "  -p <pin>  - Provide PIN credentials" << endl
       << "  -u <user> - Specify user (default admin1)" << endl;
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

