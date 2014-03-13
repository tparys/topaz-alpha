#include <unistd.h>
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

void usage();
bool require_args(int min, int passed);
uint64_t range_id_to_uid(uint64_t id);
char const *key_uid_to_str(uint64_t uid);
char const *key_mode_to_str(uint64_t mode);
uint64_t get_uid(char const *user_str);
uint64_t get_max_lba_ranges(drive &target);
void query_acct(drive &target, uint64_t uid, char const *name, int num);
void query_range(drive &target, uint64_t id);
void lock_ctl(drive &target, uint64_t id, bool on_reset, bool rd_lock, bool wr_lock);
void range_ctl(drive &target, uint64_t id, uint64_t first, uint64_t last);
void wipe_range(drive &target, uint64_t id);

int main(int argc, char **argv)
{
  atom cur_pin, new_pin;
  uint64_t user_uid = 0, range_id, start, size;
  char c;
  datum io;
  
  // Process command line switches */
  opterr = 0;
  while ((c = getopt (argc, argv, "u:p:P:n:N:v")) != -1)
  {
    switch (c)
    {
      case 'u':
	user_uid = get_uid(optarg);
	break;
	
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
  if (user_uid == 0)
  {
    cerr << "User ID not specified" << endl;
    usage();
    return -1;
  }
  
  // Open the device
  try
  {
    // Open the device
    drive target(argv[optind]);
    datum cmd, resp;
    uint64_t max_range, i;
    
    // Query pin if not yet specified
    if (cur_pin.get_type() != atom::BYTES)
    {
      cur_pin = pin_from_console("current");
    }
    
    // Login
    target.login(LOCKING_SP, user_uid, cur_pin.get_bytes());
    
    ////
    // Determine our operation
    //
    
    // Display available users
    if (strcmp(argv[optind + 1], "users") == 0)
    {
      uint64_t i;
      
      // Current admin accounts
      for (i = 1; i <= target.get_max_admins(); i++)
      {
	query_acct(target, ADMIN_BASE + i, "admin", i);
      }
      
      // Current user accounts
      for (i = 1; i <= target.get_max_users(); i++)
      {
	query_acct(target, USER_BASE + i, "user", i);
      }
    }
    // Display locking ranges
    else if (strcmp(argv[optind + 1], "ranges") == 0)
    {
      // Column headers
      cout << "Range\tCipher\tMode\tLock\t Start       Size        Last" << endl;
      max_range = get_max_lba_ranges(target);
      for (i = 0; i <= max_range; i++)
      {
	query_range(target, i);
      }
    }
    else if (strcmp(argv[optind + 1], "lock_on_reset") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	lock_ctl(target, range_id, true, true, true);
      }
    }
    else if (strcmp(argv[optind + 1], "wr_lock_on_reset") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	lock_ctl(target, range_id, true, false, true);
      }
    }
    else if (strcmp(argv[optind + 1], "unlock_on_reset") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	lock_ctl(target, range_id, true, false, false);
      }
    }
    else if (strcmp(argv[optind + 1], "lock") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	lock_ctl(target, range_id, false, true, true);
      }
    }
    else if (strcmp(argv[optind + 1], "wr_lock") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	lock_ctl(target, range_id, false, false, true);
      }
    }
    else if (strcmp(argv[optind + 1], "unlock") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	lock_ctl(target, range_id, false, false, false);
      }
    }
    else if (strcmp(argv[optind + 1], "setrange") == 0)
    {
      if (require_args(5, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	start    = atoi(argv[optind + 3]);
	size     = atoi(argv[optind + 4]);
	range_ctl(target, range_id, start, size);
      }
    }
    else if (strcmp(argv[optind + 1], "wipe") == 0)
    {
      if (require_args(3, argc - optind))
      {
	range_id = atoi(argv[optind + 2]);
	wipe_range(target, range_id);
      }
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

void usage()
{
  cerr << endl
       << "Usage:" << endl
       << "  tp_lock [opts] <drive> status    - View current Locking SP status" << endl
       << endl
       << "Options:" << endl
       << "  -p <pin>  - Provide current SID PIN" << endl
       << "  -P <file> - Read current PIN from file" << endl
       << "  -n <pin>  - Provide new SID PIN (setpin only)" << endl
       << "  -N <pin>  - Read new PIN from file (setpin only)" << endl
       << "  -v        - Increase debug verbosity" << endl;
}

bool require_args(int min, int passed)
{
  if (passed < min)
  {
    cerr << "Insufficient arguments" << endl;
    usage();
    return false;
  }
  else
  {
    return true;
  }
}

uint64_t range_id_to_uid(uint64_t id)
{
  if (id == 0)
  {
    return LBA_RANGE_GLOBAL;
  }
  else
  {
    return LBA_RANGE_BASE + id;
  }
}

char const *key_uid_to_str(uint64_t uid)
{
  if (uid == 0)
  {
    return "None";
  }
  else if (_UID_HIGH(uid) == 0x805)
  {
    return "AES-128";
  }
  else if (_UID_HIGH(uid) == 0x806)
  {
    return "AES-256";
  }
  else
  {
    return "?";
  }
}

char const *key_mode_to_str(uint64_t mode)
{
  switch (mode)
  {
    case 0:
      return "ECB";
    case 1:
      return "CBC";
    case 2:
      return "CFB";
    case 3:
      return "OFB";
    case 4:
      return "GCM";
    case 5:
      return "CTR";
    case 6:
      return "CCM";
    case 7:
      return "XTS";
    case 8:
      return "LRW";
    case 9:
      return "EME";
    case 10:
      return "CMC";
    case 11:
      return "XEX";
    default:
      return "Reserved";
  }
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

uint64_t get_max_lba_ranges(drive &target)
{
  return target.table_get(LOCKINGINFO, 4).get_uint();
}

void query_acct(drive &target, uint64_t uid, char const *name, int num)
{
  atom col;
  
  // Username
  cout << name << num << '\t';
  
  // Enabled/Disabled
  col = target.table_get(uid, 5);
  cout << (col.get_uint() ? "Enabled  " : "Disabled ");
  
  // Common name
  col = target.table_get(uid, 2);
  if ((col.get_type() == atom::BYTES) && (col.size() != 0))
  {
    col.print();
  }
  cout << endl;
}

void query_range(drive &target, uint64_t id)
{
  datum io, val;
  uint64_t key_uid, key_mode, start, size, last;
  
  // LBA_Range.GET[[]] (Everything and everything)
  io.object_uid() = range_id_to_uid(id);
  io.method_uid() = GET;
  io[0] = datum(datum::LIST);
  target.sendrecv(io);
  
  // Range ID
  cout << (int)id;
  if (id == 0)
    cout << "(G)"; // Range #0 is global
  cout << '\t';
  
  // Key type
  key_uid = io[0].find_by_name(10).value().get_uid();
  cout << key_uid_to_str(key_uid) << '\t';
  
  // Block cipher mode
  if (key_uid)
  {
    key_mode = target.table_get(key_uid, 4).get_uint();
    cout << key_mode_to_str(key_mode) << '\t';
  }
  else
  {
    cout << "None\t";
  }
  
  // Read Lock State
  if (io[0].find_by_name(5).value().get_uint())
  {
    // Read lock is enabled
    if (io[0].find_by_name(7).value().get_uint())
    {
      // And is currently on
      cout << 'R';
    }
    else
    {
      // And is currently off
      cout << 'r';
    }
  }
  else
  {
    // Disabled
    cout << '-';
  }
  
  // Write Lock State
  if (io[0].find_by_name(6).value().get_uint())
  {
    // Write lock is enabled
    if (io[0].find_by_name(8).value().get_uint())
    {
      // And is currently on
      cout << 'W';
    }
    else
    {
      // And is currently off
      cout << 'w';
    }
  }
  else
  {
    // Disabled
    cout << '-';
  }
  cout << '\t';
  
  // Start(3) and Size(4) of LBA Range
  start = io[0].find_by_name(3).value().get_uint();
  size  = io[0].find_by_name(4).value().get_uint();
  
  // Figure out last sector of range
  last = (size ? start + size - 1 : 0);
  
  // Left justify
  cout << std::left;
  
  // Output
  cout << ' ' << setw(11) << start
       << ' ' << setw(11) << size
       << ' ' << setw(11) << last;
  
  // Reset justification and finish
  cout << std::right << endl;
}

void lock_ctl(drive &target, uint64_t id, bool on_reset, bool rd_lock, bool wr_lock)
{
  datum io;
  uint64_t col_base;
  
  // Determine what locking configuration to set
  if (on_reset)
  {
    // Permanent State
    col_base = 5;
  }
  else
  {
    // Current / Temporary State
    col_base = 7;
  }
  
  // LBA_Range.SET[]
  io.object_uid()                      = range_id_to_uid(id);
  io.method_uid()                      = SET;
  io[0].name()                         = atom::new_uint(1);
  io[0].named_value()[0].name()        = atom::new_uint(col_base + 0);
  io[0].named_value()[0].named_value() = atom::new_uint(rd_lock);
  io[0].named_value()[1].name()        = atom::new_uint(col_base + 1);
  io[0].named_value()[1].named_value() = atom::new_uint(wr_lock);
  
  // Off it goes
  target.sendrecv(io);
}

void range_ctl(drive &target, uint64_t id, uint64_t first, uint64_t last)
{
  datum io;
  uint64_t size = last + 1 - first;
  
  // Range.SET[]
  io.object_uid()                      = range_id_to_uid(id);
  io.method_uid()                      = SET;
  io[0].name()                         = atom::new_uint(1);
  io[0].named_value()[0].name()        = atom::new_uint(3);     // Start
  io[0].named_value()[0].named_value() = atom::new_uint(first);
  io[0].named_value()[1].name()        = atom::new_uint(4);     // Length
  io[0].named_value()[1].named_value() = atom::new_uint(size);
  
  // Off it goes
  target.sendrecv(io);
}

void wipe_range(drive &target, uint64_t id)
{
  datum io;
  uint64_t range_uid, key_uid;
  
  // UID of desired range
  range_uid = range_id_to_uid(id);
  
  // Associated Key UID of range
  key_uid = target.table_get(range_uid, 10).get_uid();
  
  // Range.GENKEY[]
  io.object_uid() = key_uid;
  io.method_uid() = GENKEY;
  target.sendrecv(io);
}
