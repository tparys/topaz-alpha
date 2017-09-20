/**
 * Topaz Tools - Admin SP Console Utility
 *
 * Console utility to manipulate administrative features within a TCG Opal
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
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>
#include <topaz/uid.h>
#include <topaz/pin_entry.h>
using namespace std;
using namespace topaz;

void usage();
char const *lifecycle_to_string(uint64_t val);
void do_auth_login(drive &target, string pin, bool pin_valid);

int main(int argc, char **argv)
{
    string cur_pin, new_pin;
    bool cur_pin_valid = false, new_pin_valid = false;
    int c;

    // Process command line switches */
    opterr = 0;
    while ((c = getopt (argc, argv, "p:P:n:N:v")) != -1)
    {
        switch (c)
        {
            case 'p':
                cur_pin = optarg;
                cur_pin_valid = true;
                break;

            case 'P':
                cur_pin = pin_from_file(optarg);
                cur_pin_valid = true;
                break;

            case 'n':
                new_pin = optarg;
                new_pin_valid = true;
                break;

            case 'N':
                new_pin = pin_from_file(optarg);
                new_pin_valid = true;
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
        target.login_anon(ADMIN_SP);

        // Determine our operation

        // Manufacturer Default Security Identifier (MSID)
        if (strcmp(argv[optind + 1], "msid") == 0)
        {
            // Dump default PIN
            cout << target.default_pin() << endl;
        }
        else if (strcmp(argv[optind + 1], "cert") == 0)
        {
            // Dump drive security certificate
            cout << target.get_certificate() << endl;
        }
        else if (strcmp(argv[optind + 1], "status") == 0)
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
            do_auth_login(target, cur_pin, cur_pin_valid);

            // Let user know status
            cout << "Login credentials OK" << endl;
        }
        // Change SID(Admin) PIN
        else if (strcmp(argv[optind + 1], "setpin") == 0)
        {
            // Authorized session needed
            do_auth_login(target, cur_pin, cur_pin_valid);

            // Ensure new pin was provided
            if (!new_pin_valid)
            {
                // No, query for one now
                new_pin = pin_from_console_check("new SID(admin)");
            }

            // Convert PIN to atom for table I/O
            atom new_pin_atom = atom::new_bin(new_pin.c_str());

            // Set PIN of SID (Drive Owner) in Admin SP
            target.table_set(C_PIN_SID, 3, new_pin_atom);
        }
        // Activate Locking SP
        else if (strcmp(argv[optind + 1], "activate") == 0)
        {
            // Authorized session needed
            do_auth_login(target, cur_pin, cur_pin_valid);

            // Locking_SP.Activate[]
            target.invoke(LOCKING_SP, ACTIVATE);
        }
        // Revert TPer (Admin & anything else)
        else if (strcmp(argv[optind + 1], "revert") == 0)
        {
            // Authorized session needed
            do_auth_login(target, cur_pin, cur_pin_valid);

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

void usage()
{
    cerr << endl
         << "Usage:" << endl
         << "  tp_admin [opts] <drive> msid     - View MSID (default admin PIN)" << endl
         << "  tp_admin [opts] <drive> cert     - View drive security certificate" << endl
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
void do_auth_login(drive &target, string pin, bool pin_valid)
{
    // Was a PIN provided?
    if (pin_valid)
    {
        // Try and use that
        target.login(ADMIN_SP, SID, pin);
        return;
    }

    // Failing that, try the default PIN
    pin = target.default_pin();
    try
    {
        target.login(ADMIN_SP, SID, pin);
        return;
    }
    catch (topaz_exception &e)
    {
        // Didn't work
    }

    // Last effort - prompt user for PIN to use ...
    pin = pin_from_console("SID(admin)");
    target.login(ADMIN_SP, SID, pin);
}
