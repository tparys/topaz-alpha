/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Hard Drive Interface
 *
 * This file implements high level APIs used to communicate with compatible
 * TCG Opal hard drives.
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


#include <cstdio>
#include <cstring>
#include <endian.h>
#include <topaz/defs.h>
#include <topaz/debug.h>
#include <topaz/drive.h>
#include <topaz/exceptions.h>

#define PAD_TO_MULTIPLE(val, mult) (((val + (mult - 1)) / mult) * mult)

/**
 * \brief Topaz Hard Drive Constructor
 *
 * @param path OS path to specified drive (eg - '/dev/sdX')
 */
topaz::drive::drive(char const *path)
  : raw(path)
{
  // Initialization
  has_opal1 = false;
  has_opal2 = false;
  lba_align = 1;
  com_id = 0;
  
  // Check for drive TPM
  probe_tpm();
  
  // Level 0 Discovery tells us about Opal support ...
  probe_level0();
  
  // If we can, make sure we're starting from a blank slate
  if (has_opal2) reset_comid(com_id);
  
  // Query Opal Comm Properties
  probe_comm_props();
}

/**
 * \brief Topaz Hard Drive Destructor
 */
topaz::drive::~drive()
{
  // Nada
}

/**
 * \brief Combined I/O to TCG Opal drive
 *
 * @param outbuf Outbound data buffer
 */
void topaz::drive::sendrecv(topaz::datum const &outbuf, topaz::datum &inbuf)
{
  // Combined I/O - send and receive
  send(outbuf);
  recv(inbuf);
}

/**
 * \brief Send payload to TCG Opal drive
 *
 * @param outbuf Outbound data buffer
 */
void topaz::drive::send(topaz::datum const &outbuf)
{
  unsigned char *block, *payload;
  opal_header_t *header;
  size_t sub_size, pkt_size, com_size, tot_size;
  
  // Sub Packet contains the actual data
  sub_size = outbuf.size();
  
  // Packet includes Sub Packet header
  pkt_size = sub_size + sizeof(opal_sub_packet_header_t);
  
  // ... and is also padded to multiple of 4 bytes
  pkt_size = PAD_TO_MULTIPLE(pkt_size, 4);
  
  // Comm Packet includes Packet header
  com_size = pkt_size + sizeof(opal_packet_header_t);
  
  // Grand total includes last header
  tot_size = com_size + sizeof(opal_com_packet_header_t);
  
  // ... and gets padded to multiple of 512 bytes
  tot_size = PAD_TO_MULTIPLE(tot_size, ATA_BLOCK_SIZE);
  
  // Allocate some mem to work with
  block = new unsigned char[tot_size];
  
  // Set up pointers
  header = (opal_header_t*)block;
  payload = block + sizeof(opal_header_t);
  
  // Clear it out
  memset(block, 0, tot_size);
  
  // Fill in headers
  header->com_hdr.com_id = htobe16(com_id);
  header->com_hdr.length = htobe32(com_size);
  header->pkt_hdr.length = htobe32(pkt_size);
  header->sub_hdr.length = htobe32(sub_size);
  
  // Copy over payload data
  outbuf.encode_bytes(payload);
  
  // Hand off formatted Com Packet
  raw.if_send(1, com_id, block, tot_size / ATA_BLOCK_SIZE);
  
  // Cleanup
  delete [] block;
}

/**
 * \brief Receive payload from TCG Opal drive
 *
 * @param inbuf Inbound data buffer
 */
void topaz::drive::recv(topaz::datum &inbuf)
{
  unsigned char block[ATA_BLOCK_SIZE] = {0}, *payload;
  opal_header_t *header;
  size_t count;
  
  // Set up pointers
  header = (opal_header_t*)block;
  payload = block + sizeof(opal_header_t);
  
  // Receive formatted Com Packet
  raw.if_recv(1, com_id, block, 1);
  
  // Do some cursory verification here
  if (be16toh(header->com_hdr.com_id) != com_id)
  {
    throw topaz::exception("Unexpected ComID in drive response");
  }
  if (be32toh(header->com_hdr.length) <= sizeof(opal_header_t))
  {
    throw topaz::exception("Invalid Com Packet length in drive response");
  }
  
  // Ready the receiver buffer
  count = be32toh(header->sub_hdr.length);
  
  // Decode response
  inbuf.decode_bytes(payload, count);
}

/**
 * \brief Probe Available TPM Security Protocols
 */
void topaz::drive::probe_tpm()
{
  topaz::tpm_protos_t protos;
  int i, count;
  bool has_opal = false;
  
  // TPM protocols listed by IF-RECV
  TOPAZ_DEBUG(1) printf("Probe TPM Security Protocols\n");
  raw.if_recv(0, 0, &protos, 1);
  
  // Browse results
  count = be16toh(protos.list_len);
  for (i = 0; i < count; i++)
  {
    int proto = protos.list[i];
    
    // Ultimately, the only one we really need is 0x01
    if (proto == 0x01)
    {
      has_opal = true;
    }
    
    // Though verbose output is also helpful
    TOPAZ_DEBUG(2)
    {
      printf("  (0x%02x) %s\n", proto, lookup_tpm_proto(proto));
    }
  }
  
  // Verify we found 0x01
  if (!has_opal)
  {
    throw topaz::exception("Drive does not support TCG Opal");
  }
}

/**
 * \brief Probe TCG Opal Level0 Discovery
 */
void topaz::drive::probe_level0()
{
  char data[ATA_BLOCK_SIZE], *feat_data;
  topaz::level0_header_t *header = (topaz::level0_header_t*)data;
  topaz::level0_feat_t *feat;
  uint32_t total_len;
  uint16_t major, minor, code;
  size_t offset = sizeof(topaz::level0_header_t);
  
  // Level0 Discovery over IF-RECV
  TOPAZ_DEBUG(1) printf("Probe Level0 Discovery\n");
  raw.if_recv(1, 1, &data, 1);
  total_len = 4 + be32toh(header->length);
  major = be16toh(header->major_ver);
  minor = be16toh(header->minor_ver);
  TOPAZ_DEBUG(2)
  {
    printf("  Level0 Size: %d\n",  total_len);
    printf("  Level0 Version: %d / %d\n", major, minor);
  }
  
  // Verify major / minor number of structure
  if ((major != 0) || (minor != 1))
  {
    throw topaz::exception("Unexpected Level0 Revision");
  }
  
  // Tick through returned feature descriptors
  for (offset = sizeof(topaz::level0_header_t);
       offset < (total_len - sizeof(topaz::level0_feat_t));
       offset += feat->length)
  {
    // Set pointer to feature
    feat = (topaz::level0_feat_t*)(data + offset);
    
    // Move to offset of feature data
    offset += sizeof(topaz::level0_feat_t);
    feat_data = data + offset;
    
    // Rip it open
    code = be16toh(feat->code);
    TOPAZ_DEBUG(2) printf("  Feature 0x%04x v%d (%d bytes): ", code,
			  feat->version >> 4, feat->length);
    if (code == topaz::FEAT_TPER)
    {
      TOPAZ_DEBUG(2)
      {
	printf("Trusted Peripheral (TPer)\n");
	printf("    Sync: %d\n",        0x01 & (data[offset]     ));
	printf("    Async: %d\n",       0x01 & (data[offset] >> 1));
	printf("    Ack/Nak: %d\n",     0x01 & (data[offset] >> 2));
	printf("    Buffer Mgmt: %d\n", 0x01 & (data[offset] >> 3));
	printf("    Streaming: %d\n",   0x01 & (data[offset] >> 4));
	printf("    ComID Mgmt: %d\n",  0x01 & (data[offset] >> 6));
      }
    }
    else if (code == topaz::FEAT_LOCK)
    {
      TOPAZ_DEBUG(2)
      {
	printf("Locking\n");
	printf("    Supported: %d\n",        0x01 & (data[offset]     ));
	printf("    Enabled: %d\n",          0x01 & (data[offset] >> 1));
	printf("    Locked: %d\n",           0x01 & (data[offset] >> 2));
	printf("    Media Encryption: %d\n", 0x01 & (data[offset] >> 3));
	printf("    MBR Enabled: %d\n",      0x01 & (data[offset] >> 4));
	printf("    MBR Done: %d\n",         0x01 & (data[offset] >> 5));
      }
    }
    else if (code == topaz::FEAT_GEO)
    {
      topaz::feat_geo_t *geo = (topaz::feat_geo_t*)feat_data;
      lba_align = be64toh(geo->lowest_align);
      TOPAZ_DEBUG(2)
      {
	printf("Geometry Reporting\n");
	printf("    Align Required: %d\n",     0x01 & geo->align); 
	printf("    LBA Size: %d\n",           be32toh(geo->lba_size));
	printf("    Align Granularity: %ld\n", be64toh(geo->align_gran));
	printf("    Lowest Align: %lu\n",      lba_align);
      }
    }
    else if (code == topaz::FEAT_OPAL1)
    {
      topaz::feat_opal1_t *opal1 = (topaz::feat_opal1_t*)feat_data;
      has_opal1 = true;
      lba_align = 1;     // Opal 1.0 doesn't work on large sector drives
      com_id = be16toh(opal1->comid_base);
      TOPAZ_DEBUG(2)
      { 
	printf("Opal SSC 2.0\n");
	printf("    Base ComID: %u\n",            com_id);
	printf("    Number of ComIDs: %d\n",      be16toh(opal1->comid_count));
	printf("    Range cross BHV: %d\n",       0x01 & (opal1->range_bhv));
      }
    }
    else if (code == topaz::FEAT_SINGLE)
    {
      TOPAZ_DEBUG(2)
      {
	topaz::feat_single_t *single = (topaz::feat_single_t*)feat_data;
	printf("Single User Mode\n");
	printf("    Locking Objects Supported: %d\n", be32toh(single->lock_obj_count));
	printf("    Single User Presence: ");
	switch (0x03 & single->bitmask)
	{
	  case 0:
	    printf("None\n");
	    break;
	    
	  case 1:
	    printf("Some\n");
	    break;
	    
	  default:
	    printf("All\n");
	    break;
	}
	printf("    Ownership Policy: %s\n",
	       (0x04 & single->bitmask ? "Admin" : "User"));
      }
    }
    else if (code == topaz::FEAT_TABLES)
    {
      TOPAZ_DEBUG(2)
      {
	topaz::feat_tables_t *tables = (topaz::feat_tables_t*)feat_data;
	printf("Additional DataStore Tables\n");
	printf("    Max Tables: %d\n",     be16toh(tables->max_tables));
	printf("    Max Table Size: %d\n", be32toh(tables->max_size));
	printf("    Table Align: %d\n",    be32toh(tables->table_align));
      }
    }
    else if (code == topaz::FEAT_OPAL2)
    {
      topaz::feat_opal2_t *opal2 = (topaz::feat_opal2_t*)feat_data;
      has_opal2 = true;
      com_id = be16toh(opal2->comid_base);
      TOPAZ_DEBUG(2)
      { 
	printf("Opal SSC 2.0\n");
	printf("    Base ComID: %u\n",            com_id);
	printf("    Number of ComIDs: %d\n",      be16toh(opal2->comid_count));
	printf("    Range cross BHV: %d\n",       0x01 & (opal2->range_bhv));
	printf("    Max SP Admin: %d\n",          be16toh(opal2->admin_count));
	printf("    Max SP User: %d\n",           be16toh(opal2->user_count));
	printf("    C_PIN_SID Initial: ");
	if (opal2->init_pin == 0x00)
	{
	  printf("C_PIN_MSID\n");
	}
	else if (opal2->init_pin == 0xff)
	{
	  printf("Vendor Defined\n");
	}
	else
	{
	  printf("Reserved (%02x)\n", opal2->init_pin);
	}
	printf("    C_PIN_SID Revert: ");
	if (opal2->revert_pin == 0x00)
	{
	  printf("C_PIN_MSID\n");
	}
	else if (opal2->revert_pin == 0xff)
	{
	  printf("Vendor Defined\n");
	}
	else
	{
	  printf("Reserved (%02x)\n", opal2->revert_pin);
	}
      }
    }
    else if ((code >= 0x1000) && (code < 0x4000))
    {
      TOPAZ_DEBUG(2) printf("SSCs");
    }
    else if (code >= 0xc000)
    {
      TOPAZ_DEBUG(2) printf("Vendor Specific\n");
    }
    else
    {
      TOPAZ_DEBUG(2) printf("Reserved\n");
    }
  }
}

/**
 * \brief Probe TCG Opal Communication Properties
 */
void topaz::drive::probe_comm_props()
{
  topaz::datum_vector args;
  topaz::datum call(0xff, 0xff01, args);
  
  // Sub Packet has one method call
  sendrecv(call, call);
  
  printf("Received %lu bytes\n", call.size());
}

/**
 * \brief Probe TCG Opal Communication Properties
 */
void topaz::drive::reset_comid(uint32_t com_id)
{
  unsigned char block[ATA_BLOCK_SIZE] = {0};
  opal_comid_req_t *cmd = (opal_comid_req_t*)block;
  opal_comid_resp_t *resp = (opal_comid_resp_t*)block;

  // Debug
  TOPAZ_DEBUG(1) printf("Reset ComID 0x%x\n", com_id);

  // Cook up the COMID management packet
  cmd->com_id = htobe16(com_id);
  cmd->req_code = htobe32(0x02);     // STACK_RESET
  
  // Hit the reset
  raw.if_send(2, com_id, block, 1);
  raw.if_recv(2, com_id, block, 1);
  
  // Check result
  if ((htobe32(resp->avail_data) != 4) || (htobe32(resp->failed) != 0))
  {
    throw topaz::exception("Cannot reset ComID");
  }
  
  // Debug
  TOPAZ_DEBUG(2) printf("  Completed\n");
}

/**
 * \brief Convert TPM Protocol ID to String
 *
 * @param proto Protocol ID 0x00 - 0xff
 * @return String representation of ID
 */
char const *topaz::drive::lookup_tpm_proto(uint8_t proto)
{
  if (proto == 0)
  {
    return "Security Protocol Discovery";
  }
  else if ((proto >= 1) && (proto <= 6))
  {
    return "TCG Opal";
  }
  else if ((proto == 0x20) || (proto == 0xef))
  {
    return "T10 (Reserved)";
  }
  else if (proto == 0xee)
  {
    return "IEEE P1667";
  }
  else if (proto >= 0xf0)
  {
    return "Vendor Specific";
  }
  else
  {
    return "Reserved";
  }
}
