#ifndef TOPAZ_ATOM_H
#define TOPAZ_ATOM_H

/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Atom
 *
 * This class implements a TCG Opal Atom, that is a base data type which
 * encompasses both integer and binary data types. This does not include other
 * tokens such as named types, lists, or method calls
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

#include <topaz/defs.h>
#include <topaz/serializable.h>

namespace topaz
{
  
  class atom : public serializable
  {
    
  public:
    
    // Enumeration of various atom types
    typedef enum
    {
      UNSET,  // Not yet set (invalid)
      UINT,   // Unsigned integer
      INT,    // Signed integer
      BYTES   // Binary data
    } type_t;
    
    // Atom byte encodings
    typedef enum
    {
      // Tiny Atoms (Integers 0x00 - 0x3F)
      TINY        = 0x00,
      TINY_SIGN   = 0x40, // Modifier to TINY_ATOM
      
      // Short Atoms (Less than 16 bytes)
      SHORT       = 0x80,
      SHORT_BIN   = 0x20, // Modifier to SHORT_ATOM
      SHORT_SIGN  = 0x10, // Modifier to SHORT_ATOM
      
      // Medium Atoms (Less than 2048 bytes)
      MEDIUM      = 0xc0,
      MEDIUM_BIN  = 0x10, // Modifier to MEDIUM_ATOM
      MEDIUM_SIGN = 0x08, // Modifier to MEDIUM_ATOM
      
      // Long Atoms (Less than 16777216 bytes)
      LONG        = 0xe0,
      LONG_BIN    = 0x02, // Modifier to LONG_ATOM
      LONG_SIGN   = 0x01, // Modifier to LONG_ATOM
    } enc_t;

    /**
     * \brief Default Constructor
     */
    atom();
    
    /**
     * \brief Unsigned Integer Constructor
     *
     * @param value Unsigned Integer to represent
     */
    atom(uint64_t value);
    
    /**
     * \brief Signed Integer Constructor
     *
     * @param value Integer to represent
     */
    atom(int64_t value);
    
    /**
     * \brief Binary (Bytes) Constructor
     *
     * @param data Buffer to represent
     * @param len  Length of buffer
     */
    atom(void const *data, size_t len);
    
    /**
     * \brief Binary (Bytes) Constructor
     *
     * @param data Container to represent
     */
    atom(byte_vector data);
    
    /**
     * \brief Destructor
     */
    ~atom();
    
    /**
     * \brief Query Atom Type
     *
     * @return Type of atom
     */
    atom::type_t get_type();
    
    /**
     * \brief Query Encoded Header Size
     *
     * @return Size of encoded header
     */
    size_t get_header_size();
    
    /**
     * \brief Get Unsigned Integer Value
     */
    uint64_t get_uint();
    
    /**
     * \brief Get Signed Integer Value
     */
    int64_t get_int();
    
    /**
     * \brief Get Binary Data
     */
    byte_vector get_bytes();
    
  protected:
    
    /**
     * \brief Decode internal storage
     */
    void decode();
    
    /**
     * \brief Ensure data to be decoded is of minimum size
     *
     * @param min Minimum size required for buffer
     */
    void decode_check_size(size_t min);
    
    /**
     * \brief Set atom type based on bin and sign flags
     *
     * @param bits Binary / Sign flags (values 0-3)
     */
    void decode_set_type(uint8_t bits);
    
    /**
     * \brief Decode unsigned / signed integer
     */
    void decode_int();
    
    /**
     * \brief Encode Atom Bytes
     *
     * @param data Pointer to bytes to encode
     * @param len  Number of bytes to encode
     */
    void encode_atom(void const *data, size_t len);
    
    // Internal Data (raw bytes stored in std::vector superclass)
    atom::type_t data_type; // What type of data is stored
    size_t head_bytes;      // Number of bytes used by header
    union
    {
      uint64_t uint_val;    // Decoded unsigned integer value
      int64_t  int_val;     // Decoded integer value
    };
    
  };

};

#endif
