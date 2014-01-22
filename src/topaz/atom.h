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
 * encompasses both integer and binary data types, but does NOT include other
 * data stream tokens such as those for named types, lists, or method calls.
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

#include <string>
#include <topaz/defs.h>
#include <topaz/encodable.h>

namespace topaz
{
  
  class atom : public encodable
  {
    
  public:
    
    // Valid atom types
    typedef enum
    {
      EMPTY,
      UINT,   // Unsigned integer
      INT,    // Signed integer
      BYTES   // Binary data
    } type_t;
      
    // Valid atom encodings
    typedef enum
    {
      NONE,   // EMPTY - No valid atom
      TINY,   // Integers 6 bits or less
      SHORT,  // Data < 16 bytes
      MEDIUM, // Data < 2048 bytes
      LONG    // Data < 16777216 bytes
    } enc_t;
    
    // Valid tokens for encoding atoms
    typedef enum
    {
      // Tiny Atoms
      TINY_TOK    = 0x00,
      TINY_SIGN   = 0x40, // Modifier to TINY_ATOM
      
      // Short Atoms
      SHORT_TOK   = 0x80,
      SHORT_BIN   = 0x20, // Modifier to SHORT_ATOM
      SHORT_SIGN  = 0x10, // Modifier to SHORT_ATOM
      
      // Medium Atoms
      MEDIUM_TOK  = 0xc0,
      MEDIUM_BIN  = 0x10, // Modifier to MEDIUM_ATOM
      MEDIUM_SIGN = 0x08, // Modifier to MEDIUM_ATOM
      
      // Long Atoms
      LONG_TOK    = 0xe0,
      LONG_BIN    = 0x02, // Modifier to LONG_ATOM
      LONG_SIGN   = 0x01, // Modifier to LONG_ATOM
      
      // Empty
      EMPTY_TOK   = 0xff
    } tokens_t;
    
    /**
     * \brief Default Constructor
     */
    atom();
    
    /**
     * \brief Destructor
     */
    ~atom();
    
    /**
     * \brief Factory Method - Unsigned Int
     */
    static topaz::atom new_int(int64_t val);
    
    /**
     * \brief Factory Method - Unsigned Int
     */
    static topaz::atom new_uint(uint64_t val);
    
    /**
     * \brief Factory Method - Unique ID
     */
    static topaz::atom new_uid(uint64_t val);
    
    /**
     * \brief Factory Method - Binary Data
     */
    static topaz::atom new_bin(byte const *data, size_t len);
    
    /**
     * \brief Factory Method - Binary Data
     */
    static topaz::atom new_bin(byte_vector data);
    
    /**
     * \brief Equality Operator
     *
     * @return True when equal
     */
    bool operator==(atom const &ref);
    
    /**
     * \brief Inequality Operator
     *
     * @return True when not equal
     */
    bool operator!=(atom const &ref);
    
    /**
     * \brief Query encoded size
     *
     * @return Byte count of object when encoded
     */
    virtual size_t size() const;
    
    /**
     * \brief Encode to data buffer
     *
     * @param data Data buffer of at least size() bytes
     * @return Number of bytes encoded
     */
    virtual size_t encode_bytes(byte *data) const;
    
    /**
     * \brief Decode from data buffer
     *
     * @param data Location to read encoded bytes
     * @param len  Length of buffer
     * @return Number of bytes processed
     */
    virtual size_t decode_bytes(byte const *data, size_t len);
    
    /**
     * \brief Query Atom Type
     *
     * @return Type of atom
     */
    atom::type_t get_type() const;
    
    /**
     * \brief Query Atom Encoding
     *
     * @return Encoding of atom
     */
    atom::enc_t get_enc() const;
    
    /**
     * \brief Query Encoded Header Size
     *
     * @return Size of encoded header
     */
    size_t get_header_size() const;
    
    /**
     * \brief Get Unsigned Integer Stored as UID (Bytes)
     */
    uint64_t get_uid() const;
    
    /**
     * \brief Get Unsigned Integer Value
     */
    uint64_t get_uint() const;
    
    /**
     * \brief Get Signed Integer Value
     */
    int64_t get_int() const;
    
    /**
     * \brief Get Binary Data
     */
    byte_vector const &get_bytes() const;
    
    /**
     * \brief Get String
     */
    std::string get_string() const;
    
    /**
     * \brief Debug print
     */
    virtual void print() const;
    
  protected:
    
    /**
     * \brief Pick appropriate atom encoding
     *
     * @param byte_count Number of bytes
     */
    void pick_encoding(size_t byte_count);
    
    /**
     * \brief Ensure data to be decoded is of minimum size
     *
     * @param len Length of buffer
     * @param min Minimum size required for buffer
     */
    void decode_check_size(size_t len, size_t min) const;
    
    /**
     * \brief Set atom type based on bin and sign flags
     *
     * @param bits Binary / Sign flags (values 0-3)
     */
    void decode_set_type(uint8_t bits);
    
    /**
     * \brief Decode unsigned / signed integer
     *
     * @param data Pointer to binary integer data
     * @param len  Length of binary integer data
     */
    void decode_int(byte const *data, size_t len);
    
    // Internal Data (raw bytes stored in std::vector superclass)
    atom::type_t data_type; // What type of data is stored
    atom::enc_t  data_enc;  // How does it get encoded?
    size_t int_skip;        // If integer, how many bytes can be skipped
    union
    {
      uint64_t uint_val;    // Decoded unsigned integer value
      int64_t  int_val;     // Decoded integer value
    };
    byte_vector bytes;      // Container for binary bytes
    
  };

};

#endif
