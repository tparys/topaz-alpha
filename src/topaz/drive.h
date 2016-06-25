#ifndef TOPAZ_DRIVE_H
#define TOPAZ_DRIVE_H

/**
 * Topaz - Hard Drive Interface
 *
 * This file implements high level APIs used to communicate with compatible
 * TCG Opal hard drives.
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

#include <string>
#include <topaz/rawdrive.h>
#include <topaz/datum.h>

namespace topaz
{

  class drive
  {
    
    public:
    
    /**
     * \brief Topaz Hard Drive Constructor
     *
     * @param path OS path to specified drive (eg - '/dev/sdX')
     */
    drive(char const *path);
    
    /**
     * \brief Topaz Hard Drive Destructor
     */
    ~drive();
    
    /**
     * \brief Query max number of Admins in Locking SP
     */
    uint64_t get_max_admins();
    
    /**
     * \brief Query max number of users in Locking SP
     */
    uint64_t get_max_users();
    
    /**
     * \brief Combined I/O to TCG Opal drive
     *
     * @param sp_uid Target Security Provider for session (ADMIN_SP / LOCKING_SP)
     */
    void login_anon(uint64_t sp_uid);
    
    /**
     * \brief Combined I/O to TCG Opal drive
     *
     * @param sp_uid Target Security Provider for session (ADMIN_SP / LOCKING_SP)
     * @param user_uid 
     */
    void login(uint64_t sp_uid, uint64_t auth_uid, std::string pin);
    
    /**
     * \brief Query Whole Table
     *
     * @param tbl_uid Identifier of target table
     * @return Queried parameters
     */
    datum table_get(uint64_t tbl_uid);
    
    /**
     * \brief Query Value from Specified Table
     *
     * @param tbl_uid Identifier of target table
     * @param tbl_col Column number of data to retrieve (table specific)
     * @return Queried parameter
     */
    atom table_get(uint64_t tbl_uid, uint64_t tbl_col);
    
    /**
     * \brief Get Binary Table
     *
     * @param tbl_uid Identifier of target table
     * @param offset Starting byte of binary table
     * @param ptr Pointer to copy data to
     * @param len Length of data to copy
     */
    void table_get_bin(uint64_t tbl_uid, uint64_t offset,
                       void *ptr, uint64_t len);
    
    /**
     * \brief Set Value in Specified Table
     *
     * @param tbl_uid Identifier of target table
     * @param tbl_col Column number of data to retrieve (table specific)
     * @param val Value to set in column
     */
    void table_set(uint64_t tbl_uid, uint64_t tbl_col, datum val);
    
    /**
     * \brief Set Unsigned Value in Specified Table
     *
     * @param tbl_uid Identifier of target table
     * @param tbl_col Column number of data to retrieve (table specific)
     * @param val Value to set in column
     */
    void table_set(uint64_t tbl_uid, uint64_t tbl_col, uint64_t val);
    
    /**
     * \brief Set Binary Table
     *
     * @param tbl_uid Identifier of target table
     * @param offset Starting byte of binary table
     * @param ptr Pointer to copy data from
     * @param len Length of data to copy
     */
    void table_set_bin(uint64_t tbl_uid, uint64_t offset,
                       void const *ptr, uint64_t len);
    
    /**
     * \brief Set Binary Table from File
     *
     * @param tbl_uid Identifier of target table
     * @param offset Starting byte of binary table
     * @param filename File to use for input
     */
    void table_set_bin_file(uint64_t tbl_uid, uint64_t offset,
                            char const *filename);
    
    /**
     * \brief Retrieve default device PIN
     */
    std::string default_pin();
    
    /**
     * \brief Method invocation
     *
     * \param object_uid UID indicating object to use for invocation
     * \param method_uid UID indicating method to call on object
     * \param params List datum with parameters for method call
     * \return Any data returned from method call
     */
    datum invoke(uint64_t object_uid, uint64_t method_uid,
                 datum params = datum(datum::LIST));
    
    /**
     * \brief Invoke Revert[] on Admin_SP, and handle session termination
     */
    void admin_sp_revert();

    protected:
    
    /**
     * \brief Send payload to TCG Opal drive
     *
     * @param outbuf Outbound data buffer
     * \param session_ids Include TPer session IDs in ComPkt?
     */
    void send(byte_vector const &outbuf, bool session_ids = true);
    
    /**
     * \brief Receive payload from TCG Opal drive
     *
     * @param inbuf Inbound data buffer
     */
    void recv(byte_vector &inbuf);
    
    /**
     * \brief Probe Available TPM Security Protocols
     */
    void probe_tpm();
    
    /**
     * \brief Level 0 Probe - Discovery
     */
    void probe_level0();
    
    /**
     * \brief Level 1 Probe - Host Properties
     */
    void probe_level1();

    /**
     * \brief End TPM session
     */
    void logout();
    
    /**
     * \brief Read Level0 Data for Old SSC Feature Data
     */
    void parse_level0_feat_ssc1(void const *feat_data);
    
    /**
     * \brief Read Level0 Data for Newer SSC Feature Data
     */
    void parse_level0_feat_ssc2(void const *feat_data);
    
    /**
     * \brief Probe TCG Opal Communication Properties
     */
    void reset_comid(uint32_t com_id);
    
    /**
     * \brief Convert TPM Protocol ID to String
     *
     * @param proto Protocol ID 0x00 - 0xff
     * @return String representation of ID
     */
    char const *lookup_tpm_proto(uint8_t proto);
    
    // Underlying Device implementing IF-SEND/RECV
    rawdrive raw;
    byte_vector raw_buffer;
    uint64_t max_token;
    
    // TPM session data
    uint64_t tper_session_id;
    uint64_t host_session_id;
    
    // Internal info describing drive
    swg_msg_type_t msg_type;   // Enterprise or Opal
    bool has_proto_reset;
    uint32_t com_id;
    uint64_t lba_align;
    uint64_t max_com_pkt_size;
    unsigned admin_count;
    unsigned user_count;
    
  };
  
};

#endif
