#ifndef TOPAZ_DEFS_H
#define TOPAZ_DEFS_H

/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Common Definitions
 *
 * This file defines important structures used throughout TCG Opal SSC
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

#include <stdint.h>

/********* NOTE: All Structures Listed Herein Are Big Endian *********/

/* Byte alignment */
#pragma pack(push, 1)

namespace topaz
{
  
  //////////////////////////////////////////////////////////////////////////////
  // ATA Definitions
  //
  
  // Base ATA Block Size
#define ATA_BLOCK_SIZE 512
  
  // Security Protocol per AT Attachment 8 - ATA/ATAPI Command Set
  typedef struct
  {
    char          reserved[6];
    uint16_t      list_len;
    unsigned char list[504];    // Individual protocols, 1 byte each
  } tpm_protos_t;
  
  //////////////////////////////////////////////////////////////////////////////
  // TCG Opal Level 0 Discovery
  //
  
  // TCG Opal Level 0 Discovery Header
  typedef struct
  {
    uint32_t length;      // valid data length (excluding this field)
    uint16_t major_ver;   // 0x0000
    uint16_t minor_ver;   // 0x0001
    char     reserved[8];
    char     vendor[32];  // vendor specific data
  } level0_header_t;

  // TCG Opal Level 0 Feature Descriptor
  typedef struct
  {
    uint16_t code;
    uint8_t  version; // bits 4-7 version, bits 0-3 reserved
    uint8_t  length;
  } level0_feat_t;
  
  // Enumerations of known features
  typedef enum
  {
    FEAT_TPER   = 0x0001,
    FEAT_LOCK   = 0x0002,
    FEAT_GEO    = 0x0003,
    FEAT_OPAL1  = 0x0200,
    FEAT_SINGLE = 0x0201,
    FEAT_TABLES = 0x0202,
    FEAT_OPAL2  = 0x0203
  } level0_feat_id_t;
  
  // TCG Opal Geometry Feature Data (0x003)
  typedef struct
  {
    uint8_t  align;
    uint8_t  reserved[7];
    uint32_t lba_size;     // Logical Block Size
    uint64_t align_gran;   // Alignment Granularity
    uint64_t lowest_align; // Lowest Aligned LBA
  } feat_geo_t;
  
  // TCG Opal 1.0 SSC Feature Data (0x200)
  typedef struct
  {
    uint16_t comid_base;
    uint16_t comid_count;
    uint8_t  range_bhv;   // bits 1-7 reserved
  } feat_opal1_t;
  
  // TCG Opal Single User Mode Data (0x201)
  typedef struct
  {
    uint32_t lock_obj_count; // Number of locking objects supported
    uint8_t  bitmask;        // If any objects are in single user & lock policy
  } feat_single_t;

  // TCG Opal Additional DataStore Tables (0x202)
  typedef struct
  {
    uint16_t reserved;
    uint16_t max_tables;  // Max number of DataStore Tables
    uint32_t max_size;    // Max size of all DataStore Tables
    uint32_t table_align; // DataStore Table Alignment
  } feat_tables_t;
  
  // TCG Opal 2.0 SSC Feature Data (0x203)
  typedef struct
  {
    uint16_t comid_base;
    uint16_t comid_count;
    uint8_t  range_bhv;   // bits 1-7 reserved
    uint16_t admin_count; // number locking SP admin supported
    uint16_t user_count;  // number locking SP user supported
    uint8_t  init_pin;    // Initial PIN Indicator
    uint8_t  revert_pin;  // behavior on PIN revert
  } feat_opal2_t;
  
  //////////////////////////////////////////////////////////////////////////////
  // TCG Opal Communications
  //
  
  // TCG Opal ComPacket Header
  typedef struct
  {
    uint32_t reserved;
    uint16_t com_id;
    uint16_t com_id_ext;
    uint32_t tper_left;   // size of data left in TPer
    uint32_t min_xfer;    // min recv to receive tper_left data
    uint32_t length;      // size of data in this xmit
  } opal_com_packet_header_t;

  // TCG Opal Packet Header
  typedef struct
  {
    uint64_t session;
    uint32_t seq_number;
    uint16_t reserved;
    uint16_t ack_type;
    uint32_t ack;
    uint32_t length;
  } opal_packet_header_t;
  
  // TCG Opal SubPacket Header
  typedef struct
  {
    uint8_t  reserved[6];
    uint16_t kind;
    uint32_t length;
  } opal_sub_packet_header_t;
  
  // Combined header
  typedef struct
  {
    opal_com_packet_header_t com_hdr;
    opal_packet_header_t     pkt_hdr;
    opal_sub_packet_header_t sub_hdr;
  } opal_header_t;
  
  // HANDLE_COMID_REQUEST Command
  typedef struct
  {
    uint16_t com_id;
    uint16_t com_id_ext;
    uint32_t req_code;
  } opal_comid_req_t;
  
  // HANDLE_COMID_REQUEST Response
  typedef struct
  {
    uint16_t com_id;
    uint16_t com_id_ext;
    uint32_t req_code;
    uint32_t avail_data;
    uint32_t failed;
  } opal_comid_resp_t;
  
};

/* Undo byte alignment */
#pragma pack(pop)

#endif
