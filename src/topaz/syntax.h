#ifndef TOPAZ_SYNTAX_H
#define TOPAZ_SYNTAX_H

/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - TCG Opal Communications Syntax
 *
 * This file implements the syntax used within SubPackets, to encode
 * and decode the methods and information within a TCG Opal session.
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
#include <cstdlib> // for size_t
#include <vector>

namespace topaz
{
  
  // TCG Opal Syntax Tokens
  typedef enum
  {
    // Tiny Atoms (Integers 0x00 - 0x3F)
    TOK_TINY_ATOM   = 0x00,
    TOK_TINY_SIGN   = 0x40, // Modifier to TINY_ATOM
    
    // Short Atoms (Less than 16 bytes)
    TOK_SHORT_ATOM  = 0x80,
    TOK_SHORT_BIN   = 0x20, // Modifier to SHORT_ATOM
    TOK_SHORT_SIGN  = 0x10, // Modifier to SHORT_ATOM
    
    // Medium Atoms (Less than 2048 bytes)
    TOK_MEDIUM_ATOM = 0xc0,
    TOK_MEDIUM_BIN  = 0x10, // Modifier to MEDIUM_ATOM
    TOK_MEDIUM_SIGN = 0x08, // Modifier to MEDIUM_ATOM
    
    // Long Atoms (Less than 16777216 bytes)
    TOK_LONG_ATOM   = 0xe0,
    TOK_LONG_BIN    = 0x02, // Modifier to LONG_ATOM
    TOK_LONG_SIGN   = 0x01, // Modifier to LONG_ATOM
    
    // Sequence Tokens
    TOK_START_LIST  = 0xf0,
    TOK_END_LIST    = 0xf1,
    TOK_START_NAME  = 0xf2,
    TOK_END_NAME    = 0xf3,
    
    // Control Tokens
    TOK_CALL        = 0xf8,
    TOK_END_OF_DATA = 0xf9,
    TOK_END_SESSION = 0xfa,
    TOK_START_TRANS = 0xfb,
    TOK_END_TRANS   = 0xfc,
    TOK_EMPTY       = 0xff
  } syntax_token_t;
  
  // Syntax is arbitrary sequence of bytes
  class syntax : public std::vector<unsigned char>
  {
    
  public:
  
    // Constructor / Destructor
    syntax();
    ~syntax();
    
    // High level encoding
    void encode_uid(uint64_t uid);
    void encode_method_call(uint64_t invoking_uid, uint64_t method_uid,
			    syntax &args, bool abort = false);
    
    // Token encoding
    void encode_unsigned(uint64_t val);
    void encode_signed(int64_t val);
    void encode_binary(void const *data, size_t len);

    // Low level encoding
    void encode_tiny(bool sign, uint8_t data);
    void encode_short(bool bin, bool sign, void const *data, size_t len);
    void encode_medium(bool bin, bool sign, void const *data, size_t len);
    void encode_long(bool bin, bool sign, void const *data, size_t len);
    
  };
  
};

#endif
