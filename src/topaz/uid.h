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

namespace topaz
{
  typedef enum
  {
    OBJ_SESSION_MGR   = 0x00000000000000ffULL, // Session Manager UID
    OBJ_ADMIN_SP      = 0x0000020500000001ULL, // Admin Security Provider
    OBJ_C_PIN_MSID    = 0x0000000b00008402ULL  // PIN Table
  } object_uid_t;
  
  typedef enum
  {
    MTH_PROPERTIES    = 0x000000000000ff01ULL, // Query Communication Params
    MTH_START_SESSION = 0x000000000000ff02ULL, // Open Session
    MTH_SYNC_SESSION  = 0x000000000000ff03ULL, // Terminate Session
    MTH_GET           = 0x0000000600000016ULL  // Get Table Data
  } method_uid_t;
  
};

