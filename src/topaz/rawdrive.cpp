/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Low Level Hard Drive Interface
 *
 * This file implements low level APIs used to communicate with Linux ATA
 * devices over SCSI translation layer using the SGIO ioctl.
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <topaz/debug.h>
#include <topaz/defs.h>
#include <topaz/exceptions.h>
#include <topaz/rawdrive.h>
using namespace topaz;

// Set to nonzero to use ATA12 commands
#define USE_ATA12 1

#define ARGLEBARGLE 10

/**
 * \brief Topaz Raw Hard Drive Constructor
 *
 * @param path OS path to specified drive (eg - '/dev/sdX')
 */
rawdrive::rawdrive(char const *path)
{
  // First, verify libata isn't misconfigured ...
  check_libata();
  
  // Open up device
  TOPAZ_DEBUG(1) printf("Opening %s ...\n", path);
  fd = open(path, O_RDWR);
  if (fd == -1)
  {
    throw topaz_exception("Cannot open specified device");
  }
  
  // Check the TPM
  try
  {
    check_tpm();
  }
  catch (topaz_exception &e)
  {
    // Constructor not done, destructor won't be called ...
    close(fd);
    
    // Pass it along
    throw e;
  }
  
}

/**
 * \brief Topaz Raw Hard Drive Destructor
 */
rawdrive::~rawdrive()
{
  // Cleanup
  close(fd);
}

/**
 * if_send (TCG Opal IF-SEND)
 *
 * Low level interface to send data to Drive TPM
 *
 * @param protocol Security Protocol
 * @param comid    Protocol ComId
 * @param data     Data buffer
 * @param bcount   Size of data buffer in 512 byte blocks
 */
void rawdrive::if_send(uint8_t proto, uint16_t comid,
			      void *data, uint8_t bcount)
{
  if (USE_ATA12)
  {
    // Blank ATA12 command
    unsigned char cmd[7] = {0};
    
    // Fill in TCG Opal command
    cmd[0] = proto;
    cmd[1] = bcount;
    cmd[3] = comid & 0xff;
    cmd[4] = comid >> 8;
    cmd[6] = 0x5e;         // Trusted send
    
    // Off it goes
    ata_exec_12(cmd, SG_DXFER_TO_DEV, data, bcount, 1);
  }
  else
  {
    // Blank ATA command
    ata_cmd_t cmd = {0};
    
    // Fill in TCG Opal command
    cmd.feature.low  = proto;
    cmd.count.low    = bcount;
    cmd.lba_low.high = comid & 0xff;
    cmd.command      = 0x5e;
    
    // Off it goes
    ata_exec_16(cmd, SG_DXFER_TO_DEV, data, bcount, 1);
  }
}

/**
 * if_send (TCG Opal IF-SEND)
 *
 * Low level interface to receive data from Drive TPM
 *
 * @param protocol Security Protocol
 * @param comid    Protocol ComId
 * @param data     Data buffer
 * @param bcount   Size of data buffer in 512 byte blocks
 */
void rawdrive::if_recv(uint8_t proto, uint16_t comid,
			      void *data, uint8_t bcount)
{
  if (USE_ATA12)
  {
    // Blank ATA12 command
    unsigned char cmd[7] = {0};
    
    // Fill in TCG Opal command
    cmd[0] = proto;
    cmd[1] = bcount;
    cmd[3] = comid & 0xff;
    cmd[4] = comid >> 8;
    cmd[6] = 0x5c;         // Trusted receive
    
    // Off it goes
    ata_exec_12(cmd, SG_DXFER_FROM_DEV, data, bcount, 1);
  }
  else
  {
    // Blank ATA command
    ata_cmd_t cmd = {0};
    
    // Fill in TCG Opal command
    cmd.feature.low  = proto;
    cmd.count.low    = bcount;
    cmd.lba_mid.low  = comid & 0xff;
    cmd.lba_mid.high = comid >> 8;
    cmd.command      = 0x5c;
    
    // Off it goes
    ata_exec_16(cmd, SG_DXFER_FROM_DEV, data, bcount, 1);
  }
}

/**
 * check_libata
 *
 * Check libata (Linux ATA layer) for misconfiguration.
 */
void rawdrive::check_libata()
{
  int fd;
  char in;
  
  // Best effort only - /sys may not be mounted
  TOPAZ_DEBUG(1) printf("Probe libata configuration\n");
  fd = open("/sys/module/libata/parameters/allow_tpm", O_RDONLY);
  if (fd != -1)
  {
    // File opened
    if (read(fd, &in, 1) == 1)
    {
      // Data read
      if (in == '0')
      {
	throw topaz_exception(
	  "Linux libata layer configured to block TPM calls (add libata.allow_tpm=1 to kernel args)");
      }
    }
    
    // Cleanup
    close(fd);
  }
}

/**
 * check_tpm
 *
 * Check for presence of Trusted Platform Module (TPM) in drive.
 */
void rawdrive::check_tpm()
{
  uint16_t id_data[256];
  
  /* query identity */
  get_identify(id_data);
  
  /* ATA version must be >= 8 */
  TOPAZ_DEBUG(1) printf("Searching for TPM Fingerprint\n");
  if ((get_ata_ver(id_data) < 8) || ((id_data[48] & 0xC000) != 0x4000))
  {
    /* no TPM */
    throw topaz_exception("No TPM Detected in Specified Drive");
  }
}  

/**
 * ata_identify
 *
 * Retrieve ATA IDENTIFY DEVICE information
 *
 * @param data Data buffer (512 bytes)
 */
void rawdrive::get_identify(uint16_t *data)
{
  if (USE_ATA12)
  {
    // Blank ATA12 command
    unsigned char cmd[12] = {0};
    
    // IDENTIFY DEVICE
    cmd[6] = 0xEC;
    
    // Off it goes
    TOPAZ_DEBUG(1) printf("Probe ATA Identify\n");
    ata_exec_12(cmd, SG_DXFER_FROM_DEV, data, 1, 1);
  }
  else
  {
    // Blank ATA16 command
    ata_cmd_t cmd = {0};
    cmd.command = 0xEC;
    
    // Off it goes
    TOPAZ_DEBUG(1) printf("Probe ATA Identify\n");
    ata_exec_16(cmd, SG_DXFER_FROM_DEV, data, 1, 1);
  }
  
  // Useful debug
  TOPAZ_DEBUG(2)
  {
    dump_id_string("Serial", data + 10, 20);
    dump_id_string("Firmware", data + 23, 8);
    dump_id_string("Model", data + 27, 40);
  }
}

/**
 * get_ata_ver
 *
 * Decode ATA major version
 *
 * @param data Pointer to ATA identify data
 * @return ATA major version
 */
int rawdrive::get_ata_ver(uint16_t *data)
{
  unsigned int mver;
  
  // The below will probably look familiar to Linux kernel devs ...
  
  if (data[80] == 0xFFFF)
    return 0;
  
  for (mver = 14; mver >= 1; mver--)
    if (data[80] & (1 << mver))
      break;
  
  return mver;
}

/**
 * dump_id_string
 *
 * Print a string encoded in a set of uint16_t data.
 *
 * @param data Pointer to start of uin16_t encoded string
 * @param max  Maximum size of string
 */
void rawdrive::dump_id_string(char const *desc, uint16_t *data, size_t max)
{
  size_t i;
  uint16_t word;
  char c;
  
  printf("  %s: ", desc);
  for (i = 0; i < max; i++)
  {
    word = data[i >> 1];
    
    // Toggle on high/low byte
    if (i % 2)
    {
      c = 0xff & word;
    }
    else
    {
      c = 0xff & (word >> 8);
    }
    
    // Stop on NULL
    if (c == 0x00)
    {
      break;
    }
    // Skip spaces
    if (c != ' ')
    {
      printf("%c", c);
    }
  }
  printf("\n");
}

/**
 * ata_exec_12
 *
 * Execute ATA12 command through SCSI/ATA translation layer,
 * using Linux SGIO ioctl interface.
 *
 * @param cmd    7 byte buffer to valid ATA12 command
 * @param type   IO type (SG_DXFER_NONE/SG_DXFER_FROM_DEV/SG_DXFER_TO_DEV)
 * @param data   Data buffer for ATA operation, NULL on SGIO_DATA_NONE
 * @param bcount Length of data buffer in blocks (512 bytes)
 * @param wait   Command timeout (seconds)
 */
void rawdrive::ata_exec_12(unsigned char const *cmd, int type,
				  void *data, uint8_t bcount, int wait)
{
  struct sg_io_hdr sg_io;  // ioctl data structure
  unsigned char cdb[12];   // Command descriptor block
  unsigned char sense[32]; // SCSI sense (error) data
  int rc;
  
  // Initialize structures
  memset(&sg_io, 0, sizeof(sg_io));
  memset(&cdb, 0, sizeof(cdb));
  memset(&sense, 0, sizeof(sense));
  
  ////
  // Fill in ioctl data for ATA16 pass through
  //
  
  // Mandatory per interface
  sg_io.interface_id    = 'S';
  
  // Location, size of command descriptor block (command)
  sg_io.cmdp            = cdb;
  sg_io.cmd_len         = sizeof(cdb);
  
  // Command data transfer (optional)
  sg_io.dxferp          = data;
  sg_io.dxfer_len       = bcount * ATA_BLOCK_SIZE;
  sg_io.dxfer_direction = type;
  
  // Sense (error) data
  sg_io.sbp             = sense;
  sg_io.mx_sb_len       = sizeof(sense);
  
  // Timeout (ms)
  sg_io.timeout         = wait * 1000;
  
  ////
  // Fill in SCSI command
  //
  
  // Byte 0: ATA12 pass through
  cdb[0] = 0xA1;
  
  // Byte 1: ATA protocol (read/write/none)
  // Byte 2: Check condition, blocks, size, I/O direction
  // Final direction specific bits
  switch (type)
  {
    case SG_DXFER_NONE:
      cdb[1] = 3 << 1; // ATA no data
      cdb[2] = 0x20;   // Check condition only
      break;
      
    case SG_DXFER_FROM_DEV:
      cdb[1] = 4 << 1; // ATA PIO-in
      cdb[2] = 0x2e;   // Check, blocks, size in sector count, read
      break;

    case SG_DXFER_TO_DEV:
      cdb[1] = 5 << 1; // ATA PIO-out
      cdb[2] = 0x26;   // Check, blocks, size in sector count
      break;
      
    default: // Invalid
      throw topaz_exception("Invalid ATA Direction");
      break;
  }
  
  // Rest of ATA12 command get copied here (7 bytes)
  memcpy(cdb + 3, cmd, 7);
  
  ////
  // Run ioctl
  //
  
  // Debug output command
  TOPAZ_DEBUG(4)
  {
    // Command descriptor block
    printf("ATA Command:\n");
    topaz_dump(cmd, 7);
    
    // Command descriptor block
    printf("SCSI CDB:\n");
    topaz_dump(cdb, sizeof(cdb));
    
    // Data out?
    if (type == SG_DXFER_TO_DEV)
    {
      printf("Write Data:\n");
      topaz_dump(data, bcount * ATA_BLOCK_SIZE);
    }
  }
  
  // System call
  rc = ioctl(fd, SG_IO, &sg_io);
  if (rc != 0)
  {
    throw topaz_exception("SGIO ioctl failed");
  }
  
  // Debug input
  if (type == SG_DXFER_FROM_DEV)
  {
    TOPAZ_DEBUG(4)
    {
      printf("Read Data:\n");
      topaz_dump(data, bcount * ATA_BLOCK_SIZE);
    }
  }
  
  // Check sense data
  if (sense[0] != 0x72 || sense[7] != 0x0e || sense[8] != 0x09
      || sense[9] != 0x0c || sense[10] != 0x00)
  {
    //fprintf(stderr, "error  = %02x\n", sense[11]);    // 0x00 means success
    //fprintf(stderr, "status = %02x\n", sense[21]);    // 0x50 means success
    throw topaz_exception("SGIO ioctl bad status");
  }
}

/**
 * ata_exec_16
 *
 * Execute ATA16 command through SCSI/ATA translation layer,
 * using Linux SGIO ioctl interface.
 *
 * @param cmd    12 byte buffer to valid ATA16 command
 * @param type   IO type (SG_DXFER_NONE/SG_DXFER_FROM_DEV/SG_DXFER_TO_DEV)
 * @param data   Data buffer for ATA operation, NULL on SGIO_DATA_NONE
 * @param bcount Length of data buffer in blocks (512 bytes)
 * @param wait   Command timeout (seconds)
 */
void rawdrive::ata_exec_16(ata_cmd_t &cmd, int type,
				  void *data, uint8_t bcount, int wait)
{
  struct sg_io_hdr sg_io;  // ioctl data structure
  unsigned char cdb[16];   // Command descriptor block
  unsigned char sense[32]; // SCSI sense (error) data
  int rc;
  
  // Initialize structures
  memset(&sg_io, 0, sizeof(sg_io));
  memset(&cdb, 0, sizeof(cdb));
  memset(&sense, 0, sizeof(sense));
  
  ////
  // Fill in ioctl data for ATA16 pass through
  //
  
  // Mandatory per interface
  sg_io.interface_id    = 'S';
  
  // Location, size of command descriptor block (command)
  sg_io.cmdp            = cdb;
  sg_io.cmd_len         = sizeof(cdb);
  
  // Command data transfer (optional)
  sg_io.dxferp          = data;
  sg_io.dxfer_len       = bcount * ATA_BLOCK_SIZE;
  sg_io.dxfer_direction = type;
  
  // Sense (error) data
  sg_io.sbp             = sense;
  sg_io.mx_sb_len       = sizeof(sense);
  
  // Timeout (ms)
  sg_io.timeout         = wait * 1000;
  
  ////
  // Fill in SCSI command
  //
  
  // Byte 0: ATA16 pass through
  cdb[0] = 0x85;
  
  // Byte 1: ATA protocol (read/write/none)
  // Byte 2: Check condition, blocks, size, I/O direction
  // Final direction specific bits
  switch (type)
  {
    case SG_DXFER_NONE:
      cdb[1] = 3 << 1; // ATA no data
      cdb[2] = 0x20;   // Check condition only
      break;
      
    case SG_DXFER_FROM_DEV:
      cdb[1] = 4 << 1; // ATA PIO-in
      cdb[2] = 0x2e;   // Check, blocks, size in sector count, read
      break;

    case SG_DXFER_TO_DEV:
      cdb[1] = 5 << 1; // ATA PIO-out
      cdb[2] = 0x26;   // Check, blocks, size in sector count
      break;
      
    default: // Invalid
      throw topaz_exception("Invalid ATA Direction");
      break;
  }

  // Rest of ATA16 command get copied here (12 bytes)
  memcpy(cdb + 3, &cmd, 12);
  
  ////
  // Run ioctl
  //
  
  // Debug output command
  TOPAZ_DEBUG(4)
  {
    // Command descriptor block
    printf("SCSI CDB:\n");
    topaz_dump(cdb, sizeof(cdb));
    
    // Data out?
    if (type == SG_DXFER_TO_DEV)
    {
      printf("Write Data:\n");
      topaz_dump(data, bcount * ATA_BLOCK_SIZE);
    }
  }
  
  // System call
  rc = ioctl(fd, SG_IO, &sg_io);
  if (rc != 0)
  {
    throw topaz_exception("SGIO ioctl failed");
  }
  
  // Debug input
  if (type == SG_DXFER_FROM_DEV)
  {
    TOPAZ_DEBUG(4)
    {
      printf("Read Data:\n");
      topaz_dump(data, bcount * ATA_BLOCK_SIZE);
    }
  }
  
  // Check sense data
  if (sense[0] != 0x72 || sense[7] != 0x0e || sense[8] != 0x09
      || sense[9] != 0x0c || sense[10] != 0x00)
  {
    //fprintf(stderr, "error  = %02x\n", sense[11]);    // 0x00 means success
    //fprintf(stderr, "status = %02x\n", sense[21]);    // 0x50 means success
    throw topaz_exception("SGIO ioctl bad status");
  }
}
