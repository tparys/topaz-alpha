/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Unique ID's
 *
 * This file implements the known Object and Method UIDs used within the
 * TCG Opal Protocol.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _UID_MAKE(high, low) (((high) * 0x100000000ULL) + (low))
#define _UID_HIGH(uid)       ((uid) / 0x100000000ULL)
#define _UID_LOW(uid)        ((uid) & 0x0ffffffffULL)

namespace topaz
{
  
  ////
  // Objects Defined within TCG Opal
  //
  
  // Global UIDs
  enum
  {
    // Session Manager - Start / Stop Sessions to defined SPs
    SESSION_MGR      = _UID_MAKE(   0x0,    0xff),
    
    // Admin SP - Master Control
    ADMIN_SP         = _UID_MAKE( 0x205,     0x1),
    
    // Locking SP - LBA Range Control
    LOCKING_SP       = _UID_MAKE( 0x205,     0x2)
  };
  
  // Defined UIDs within Admin SP
  enum
  {
    // Info
    SP_INFO          = _UID_MAKE(   0x1,     0x2),
    
    // Admin SP User Security Identifier (SID)
    SID              = _UID_MAKE(   0x9,     0x6),
    
    // C_PIN Tables documented in Opal 1.0 Spec - Section 4.3.1.9
    C_PIN_SID        = _UID_MAKE(   0xb,     0x1), // Current PIN of SID
    C_PIN_MSID       = _UID_MAKE(   0xb,  0x8402)  // Factory Default PIN of SID
  };
  
  // Defined UIDs within Locking SP
  enum
  {
    C_PIN_ADMIN_BASE = _UID_MAKE(   0xb, 0x10000), // Base UID of Admins (+1, +2, +3 ...)
    C_PIN_USER_BASE  = _UID_MAKE(   0xb, 0x30000), // Base UID of Users (+1, +2, +3 ...)
    
    // Master Locking SP table
    LOCKING          = _UID_MAKE( 0x802,     0x1),
    
    // LBA Ranges Objects
    LBA_RANGE_BASE   = _UID_MAKE( 0x802, 0x30001)
  };
  
  ////
  // Methods Defined within TCG Opal
  //
  
  // Method calls valid for session manager
  enum
  {
    PROPERTIES    = _UID_MAKE(0, 0xff01), // Query Communication Params
    START_SESSION = _UID_MAKE(0, 0xff02), // Open Session
    SYNC_SESSION  = _UID_MAKE(0, 0xff03)  // Terminate Session
  };
  
  // TCG Opal Method Calls
  enum
  {
    REVERT_SP     = _UID_MAKE(6,   0x11), // RevertSP
    GET           = _UID_MAKE(6,   0x16), // Get Table Data
    SET           = _UID_MAKE(6,   0x17), // Set Table Data
    REVERT        = _UID_MAKE(6,  0x202), // Revert
    ACTIVATE      = _UID_MAKE(6,  0x203)  // Activate
  };
  
  ////
  // Table Column Definitions
  //
  
  // AdminSP C_PIN_*
  enum
  {
    ASP_CPIN_UID         = 0,
    ASP_CPIN_NAME        = 1,
    ASP_CPIN_COMMON_NAME = 2
    
  };
  
};
