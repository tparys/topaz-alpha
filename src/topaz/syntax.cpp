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

#include <endian.h>
#include <topaz/exceptions.h>
#include <topaz/syntax.h>

// Syntax Constructor
topaz::syntax::syntax()
{
  // Nada
}

// Syntax Destructor
topaz::syntax::~syntax()
{
  // Nada
}

/**
 * \brief Encode Unique ID
 *
 * Encode UID as 8 byte chunk into TCG Opal data stream
 *
 * @param uid Unique ID to encode
 */
void topaz::syntax::encode_uid(uint64_t uid)
{
  uint64_t flip;
  
  // Byte flip to big endian
  flip = htobe64(uid);
  
  // Encode individual bytes to stream
  encode_binary(&flip, 8);
}

/**
 * \brief Encode Method Call
 *
 * Encode a method call into TCG Opal data stream
 *
 * @param obj_uid    Unique ID of object of invoking object
 * @param method_uid Unique ID of method to call
 * @param args       Encoded arguments to method. Note that Outermost start/end
 *                   list tokens are not needed, and will be included for you.
 * @param abort      Flag indicating that method should be aborted
 */
void topaz::syntax::encode_method_call(uint64_t obj_uid, uint64_t method_uid,
				       syntax &args, bool abort)
{
  // First the token call indicator
  push_back(TOK_CALL);
  
  // Encode Invoking UID (Object)
  encode_uid(obj_uid);
  
  // Encode Method UID
  encode_uid(method_uid);
  
  // List of arguments
  push_back(TOK_START_LIST);
  
  // Insert arg data at end of encoding
  insert(end(), args.begin(), args.end());
  
  // End of arguments
  push_back(TOK_END_LIST);
  
  // End of method call
  push_back(TOK_END_OF_DATA);
  
  // Status code list
  push_back(TOK_START_LIST);
  
  // Abort indicator
  push_back(abort ? 0x01 : 0x00);
  
  // Reserved, left at zero
  push_back(0x00);
  push_back(0x00);
  
  // End of list
  push_back(TOK_END_LIST);
}

/**
 * \brief Encode Unsigned Integer
 *
 * Encode an unsigned datatype (max 8 bytes) into TCG Opal data stream
 *
 * @param val Value to encode
 */
void topaz::syntax::encode_unsigned(uint64_t val)
{
  int skip;
  union
  {
    uint64_t flip;
    unsigned char raw[8];
  };
  
  // Really small ints fit into a single byte
  if (val < 0x40)
  {
    // Tiny Atom (single byte)
    encode_tiny(false, val);
  }
  else
  {
    // Byteflip to big endian
    flip = htobe64(val);
    
    // Drop unneeded leading zeroes (up to 7)
    for (skip = 0; (skip < 8) && (raw[skip] == 0x00); skip++) {}
    
    // Short atom holds up to 15 bytes,
    // may need to adjust if/when uint128_t are needed ...
    encode_short(false, false, raw + skip, 8 - skip);
  }
}

/**
 * \brief Encode Signed Integer
 *
 * Encode a signed datatype (max 8 bytes) into TCG Opal data stream
 *
 * @param val Value to encode
 */
void topaz::syntax::encode_signed(int64_t val)
{
  int skip;
  union
  {
    uint64_t flip;
    unsigned char raw[8];
  };
  
  // Really small ints fit into a single byte
  if ((val < 0x20) && (val >= -0x20))
  {
    // Tiny Atom (single byte)
    encode_tiny(true, val);
  }
  else
  {
    // Byteflip to big endian
    flip = htobe64(val);
    
    // Logic differs a bit based on sign
    if (val < 0)
    {
      // Negative condition is to drop 0xff bytes,
      // provided the remaining value is still negative
      // (most significant remaining bit is a 1)
      for (skip = 0; (skip < 8) && (raw[skip] == 0xff) &&
	     ((raw[skip + 1] & 0x80) == 0x80); skip++) {}
    }
    else
    {
      // Positive condition is to drop 0x00 bytes,
      // provided the remaining valus is still positivie
      // (most significant remaining bit is a 0)
      for (skip = 0; (skip < 8) && (raw[skip] == 0x00) &&
	     ((raw[skip + 1] & 0x80) == 0x00); skip++) {}
    }
    
    // Short atom holds up to 15 bytes,
    // may need to adjust if/when uint128_t are needed ...
    encode_short(false, true, raw + skip, 8 - skip);
  }
}

/**
 * \brief Encode Binary Data
 *
 * Encode binary data into TCG Opal data stream
 *
 * @param data Pointer to binary data buffer
 * @param len  Length of data buffer
 */
void topaz::syntax::encode_binary(void const *data, size_t len)
{
  if (len < 16)
  {
    encode_short(true, false, data, len);
  }
  else if (len < 2048)
  {
    encode_medium(true, false, data, len);
  }
  else
  {
    encode_long(true, false, data, len);
  }
}

/**
 * \brief Encode Tiny Atom
 *
 * Encode a datatype as a TCG Opal Tiny Atom (0x3f or less)
 *
 * @param sign Indicate integer is signed
 * @param data value 0x3f or less
 */
void topaz::syntax::encode_tiny(bool sign, uint8_t data)
{
  unsigned char byte;
  
  // First / only byte
  byte = TOK_TINY_ATOM;
  if (sign)
  {
    byte |= TOK_TINY_SIGN;
  }
  byte |= 0x3f & data;
  push_back(byte);
}

/**
 * \brief Encode Short Atom
 *
 * Encode a datatype as a TCG Opal Short Atom (15 bytes or less)
 *
 * @param bin   Indicate data is binary (else integer)
 * @param sign  If integer, indicate value is signed
 * @param data  Pointer to raw data to encode
 * @param len   Byte count of data
 */
void topaz::syntax::encode_short(bool bin, bool sign,
				 void const *data, size_t len)
{
  unsigned char byte, *cdata = (unsigned char*)data;
  
  // Sanity check
  if (len >= (1 << 6))
  {
    throw topaz::exception("Value too large for short atom");
  }
  
  // First byte
  byte = TOK_SHORT_ATOM;
  if (bin)
  {
    byte |= TOK_SHORT_BIN;
  }
  else if (sign)
  {
    byte |= TOK_SHORT_SIGN;
  }
  byte |= len;
  push_back(byte);
  
  // Add data
  for (size_t i = 0; i < len; i++)
  {
    push_back(cdata[i]);
  }
}

/**
 * \brief Encode Medium Atom
 *
 * Encode a datatype as a TCG Opal Medium Atom (2047 bytes or less)
 *
 * @param bin   Indicate data is binary (else integer)
 * @param sign  If integer, indicate value is signed
 * @param data  Pointer to raw data to encode
 * @param len   Byte count of data
 */
void topaz::syntax::encode_medium(bool bin, bool sign,
				  void const *data, size_t len)
{
  unsigned char byte, *cdata = (unsigned char*)data;
  
  // Sanity check
  if (len >= (1 << 11))
  {
    throw topaz::exception("Value too large for medium atom");
  }
  
  // First byte
  byte = TOK_MEDIUM_ATOM;
  if (bin)
  {
    byte |= TOK_MEDIUM_BIN;
  }
  else if (sign)
  {
    byte |= TOK_MEDIUM_SIGN;
  }
  byte |= 0x03 & (len >> 8);
  push_back(byte);
  
  // Second byte
  byte = 0xff & len;
  push_back(byte);
  
  // Add data
  for (size_t i = 0; i < len; i++)
  {
    push_back(cdata[i]);
  }
}

/**
 * \brief Encode Long Atom
 *
 * Encode a datatype as a TCG Opal Long Atom (16777215 bytes or less)
 *
 * @param bin   Indicate data is binary (else integer)
 * @param sign  If integer, indicate value is signed
 * @param data  Pointer to raw data to encode
 * @param len   Byte count of data
 */
void topaz::syntax::encode_long(bool bin, bool sign,
				void const *data, size_t len)
{
  unsigned char byte, *cdata = (unsigned char*)data;
  
  // Sanity check
  if (len >= (1 << 24))
  {
    throw topaz::exception("Value too large for long atom");
  }
  
  // First byte
  byte = TOK_LONG_ATOM;
  if (bin)
  {
    byte |= TOK_LONG_BIN;
  }
  else if (sign)
  {
    byte |= TOK_LONG_SIGN;
  }
  push_back(byte);
  
  // Second byte
  byte = 0xff & (len >> 16);
  push_back(byte);
  
  // Third byte
  byte = 0xff & (len >> 8);
  push_back(byte);
  
  // Fourth byte
  byte = 0xff & len;
  push_back(byte);
  
  // Add data
  for (size_t i = 0; i < len; i++)
  {
    push_back(cdata[i]);
  }
}
