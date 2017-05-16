#ifndef TOPAZ_RAWDRIVE_H
#define TOPAZ_RAWDRIVE_H

/**
 * Topaz - Low Level Hard Drive Interface
 *
 * This file implements low level APIs used to communicate with Linux ATA
 * devices over SCSI translation layer using the SGIO ioctl.
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

#include <stdint.h>
#include <stddef.h> /* size_t */
#include <string>

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

        /**
         * Get drive model number
         *
         * @return String representing model number of drive
         */
        std::string get_model() const;

        /**
         * Get drive serial number
         *
         * @return String representing serial number of drive
         */
        std::string get_serial() const;

        /**
         * Get drive firmware revision
         *
         * @return String representing firmware revision of drive
         */
        std::string get_firmware() const;

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
         * get_id_string
         *
         * Query a string encoded in a set of uint16_t data.
         *
         * @param data Pointer to start of uin16_t encoded string
         * @param max  Maximum size of string
         * @return Value of string
         */
        std::string get_id_string(uint16_t *data, size_t max);

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

        /* drive identification */
        std::string drive_model;
        std::string drive_serial;
        std::string drive_firmware;

    };

};

#endif
