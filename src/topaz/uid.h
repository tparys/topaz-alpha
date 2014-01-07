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
  typedef enum
  {
    OBJ_SESSION_MGR   = _UID_MAKE(   0x0,   0xff), // Session Manager UID
    OBJ_SID           = _UID_MAKE(   0x9,    0x6), // ??
    OBJ_C_PIN_SID     = _UID_MAKE(   0xb,    0x1), // PIN Table
    OBJ_C_PIN_MSID    = _UID_MAKE(   0xb, 0x8402), // Manufactured Default PIN Table
    OBJ_ADMIN_SP      = _UID_MAKE( 0x205,    0x1)  // Admin Security Provider
  } object_uid_t;
  
  typedef enum
  {
    MTH_PROPERTIES    = _UID_MAKE(0, 0xff01), // Query Communication Params
    MTH_START_SESSION = _UID_MAKE(0, 0xff02), // Open Session
    MTH_SYNC_SESSION  = _UID_MAKE(0, 0xff03), // Terminate Session
    MTH_REVERT_SP     = _UID_MAKE(6,   0x11), // RevertSP
    MTH_GET           = _UID_MAKE(6,   0x16), // Get Table Data
    MTH_REVERT        = _UID_MAKE(6,  0x202)  // Revert
  } method_uid_t;
  
};
