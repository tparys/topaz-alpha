#ifndef TOPAZ_RAWDRIVE_H
#define TOPAZ_RAWDRIVE_H

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

#include <stdint.h>
#include <stddef.h> /* size_t */

namespace topaz
{
  
  // MSB 
  typedef struct
  {
    uint8_t high;
    uint8_t low;
  } ata_word_t;
  
  // ATA12 Command
  typedef struct
  {
    uint8_t feature;
    uint8_t count;
    uint8_t lba_low;
    uint8_t lba_mid;
    uint8_t lba_high;
    uint8_t device;
    uint8_t command;
  } ata12_cmd_t;
  
  // ATA16 Command
  typedef struct
  {
    ata_word_t feature;
    ata_word_t count;
    ata_word_t lba_low;
    ata_word_t lba_mid;
    ata_word_t lba_high;
    uint8_t    device;
    uint8_t    command;
  } ata16_cmd_t;
  
  class rawdrive
  {
    
  public:
    
    /**
     * \brief Topaz Raw Hard Drive Constructor
     *
     * @param path OS path to specified drive (eg - '/dev/sdX')
     */
    rawdrive(char const *path);
    
    /**
     * \brief Topaz Raw Hard Drive Destructor
     */
    ~rawdrive();

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
    void if_send(uint8_t proto, uint16_t comid,
		 void *data, uint8_t bcount);
    
    /**
     * if_send (TCG Opal IF-RECV)
     *
     * Low level interface to receive data from Drive TPM
     *
     * @param protocol Security Protocol
     * @param comid    Protocol ComId
     * @param data     Data buffer
     * @param bcount   Size of data buffer in 512 byte blocks
     */
    void if_recv(uint8_t proto, uint16_t comid,
		 void *data, uint8_t bcount);
    
  protected:
    
    /**
     * check_libata
     *
     * Check libata (Linux ATA layer) for misconfiguration.
     */
    void check_libata();
    
    /**
     * check_tpm
     *
     * Check for presence of Trusted Platform Module (TPM) in drive.
     */
    void check_tpm();

    /**
     * get_identify
     *
     * Retrieve ATA IDENTIFY DEVICE information
     *
     * @param data Data buffer (512 bytes)
     */
    void get_identify(uint16_t *data);
    
    /**
     * get_ata_ver
     *
     * Decode ATA major version
     *
     * @param data Pointer to ATA identify data
     * @return ATA major version
     */
    int get_ata_ver(uint16_t *data);
    
    /**
     * dump_id_string
     *
     * Print a string encoded in a set of uint16_t data.
     *
     * @param desc String describing the encoded string
     * @param data Pointer to start of uin16_t encoded string
     * @param max  Maximum size of string
     */
    void dump_id_string(char const *desc, uint16_t *data, size_t max);
    
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
    void ata_exec_12(ata12_cmd_t &cmd, int type,
		     void *data, uint8_t bcount, int wait);
    
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
    void ata_exec_16(ata16_cmd_t &cmd, int type,
		     void *data, uint8_t bcount, int wait);
    
    /* internal data */
    int fd;
    
  };
  
};

#endif
