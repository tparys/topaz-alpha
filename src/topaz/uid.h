/**
 * Topaz - Unique ID's
 *
 * This file implements the known Object and Method UIDs used within the
 * TCG Opal Protocol.
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
    
    // An unauthenticated user (anon login)
    ANYBODY          = _UID_MAKE(   0x9,     0x1),
    
    // Admin SP User Security Identifier (SID)
    SID              = _UID_MAKE(   0x9,     0x6),
    
    // Physical Security ID
    PSID             = _UID_MAKE(   0x9, 0x1ff01),
    
    // C_PIN Tables documented in Opal 1.0 Spec - Section 4.3.1.9
    C_PIN_SID        = _UID_MAKE(   0xb,     0x1), // Current PIN of SID
    C_PIN_MSID       = _UID_MAKE(   0xb,  0x8402)  // Factory Default PIN of SID
  };
  
  // Defined UIDs within Locking SP
  enum
  {
    // Access Control Entries
    ACE_DATASTORE_GET = _UID_MAKE(  0x8,  0x3fc00), // Read access to DataStore Table
    ACE_DATASTORE_SET = _UID_MAKE(  0x8,  0x3fc01), // Write access to DataStore Table
    
    ADMINS           = _UID_MAKE(   0x9,     0x2), // ?
    ADMIN_BASE       = _UID_MAKE(   0x9, 0x10000), // Base UID of Admins (+1, +2, +3 ...)
    USER_BASE        = _UID_MAKE(   0x9, 0x30000), // Base UID of Users (+1, +2, +3 ...)
    C_PIN_ADMIN_BASE = _UID_MAKE(   0xb, 0x10000), // Base UID of Admins PIN (+1, +2, +3 ...)
    C_PIN_USER_BASE  = _UID_MAKE(   0xb, 0x30000), // Base UID of Users PIN (+1, +2, +3 ...)

    LOCKINGINFO      = _UID_MAKE( 0x801,     0x1), // LockingInfo (max ranges & such)
    LOCKING          = _UID_MAKE( 0x802,     0x1), // Locking Table (current ranges)
    
    // MBR Shadowing (Size of MBR in descriptor object, 1:804
    MBR_CONTROL      = _UID_MAKE( 0x803,     0x1), // MBR Control Register
    MBR_UID          = _UID_MAKE( 0x804,     0x0), // MBR ?
    MBR              = _UID_MAKE( 0x804,     0x1), // MBR ?

    // LBA Ranges Objects
    LBA_RANGE_GLOBAL = _UID_MAKE( 0x802,     0x1),
    LBA_RANGE_BASE   = _UID_MAKE( 0x802, 0x30000),

    // DataStore
    DATASTORE        = _UID_MAKE( 0x1001, 0),
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
    GENKEY        = _UID_MAKE(6,   0x10), // Generate new key
    REVERT_SP     = _UID_MAKE(6,   0x11), // RevertSP
    
    // Get[] - SWG Core Spec - 5.3.3.6
    // Note: cellblocks listed in Core Spec - 5.1.2.3
    GET           = _UID_MAKE(6,   0x16),
    
    // Set[] - SWG Core Spec - 5.3.3.7
    // Note: cellblocks listed in Core Spec - 5.1.2.3
    SET           = _UID_MAKE(6,   0x17),
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
