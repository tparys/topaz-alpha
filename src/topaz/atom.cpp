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

#include <cstdio>

#include <endian.h>
#include <topaz/atom.h>
#include <topaz/exceptions.h>
using namespace topaz;

#define CHECK_SIZE(x) if (size() < x) { throw topaz::exception("Truncated Atom data"); }

/**
 * \brief Default Constructor
 */
atom::atom()
{
  // Initialize
  data_type = atom::UNSET;
  head_bytes = 0;
  uint_val = 0;
}

/**
 * \brief Unsigned Integer Constructor
 *
 * @param value Unsigned Integer to represent
 */
atom::atom(uint64_t value)
{
  int skip;
  union
  {
    uint64_t flip;
    byte     raw[8];
  };
  
  // Initialize
  data_type = atom::UINT;
  uint_val = value;
  
  // Really small values fit into a single byte
  if (value < 0x40)
  {
    // Tiny atom (Data stored w/ header in same byte)
    push_back(atom::TINY | value);
    head_bytes = 0;
  }
  else // Determine how many bytes are really needed
  {
    // Byteflip to big endian
    flip = htobe64(value);
    
    // Drop unneeded leading zeroes (up to 7)
    for (skip = 0; (skip < 8) && (raw[skip] == 0x00); skip++) {}
   
    // Encode byte pattern
    encode_atom(raw + skip, 8 - skip);
  }
}

/**
 * \brief Integer Constructor
 *
 * @param value Integer to represent
 */
atom::atom(int64_t value)
{
  int skip;
  union
  {
    uint64_t flip;
    byte     raw[8];
  };
  
  // Initialize
  data_type = atom::INT;
  int_val = value;
  
  // Really small values fit into a single byte
  if ((value < 0x20) && (value >= -0x20))
  {
    // Tiny atom (Data stored w/ header in same byte)
    push_back(atom::TINY | atom::TINY_SIGN | (0x3f & value));
    head_bytes = 0;
  }
  else // Determine how many bytes are really needed
  {
    // Byteflip to big endian
    flip = htobe64(value);
    
    // Logic differs a bit based on sign
    if (value < 0)
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
      // provided the remaining value is still positive
      // (most significant remaining bit is a 0)
      for (skip = 0; (skip < 8) && (raw[skip] == 0x00) &&
	     ((raw[skip + 1] & 0x80) == 0x00); skip++) {}
    }
    
    // Encode byte pattern
    encode_atom(raw + skip, 8 - skip);
  }
}

/**
 * \brief Binary (Bytes) Constructor
 *
 * @param data Buffer to represent
 * @param len  Length of buffer
 */
atom::atom(void const *data, size_t len)
{
  // Intialize
  data_type = atom::BYTES;
  encode_atom(data, len);
}

/**
 * \brief Binary (Bytes) Constructor
 *
 * @param data Container to represent
 */
atom::atom(byte_vector data)
{
  // Intialize
  data_type = atom::BYTES;
  encode_atom(&(data[0]), data.size());
}

/**
 * \brief Destructor
 */
atom::~atom()
{
  // Nada
}

/**
 * \brief Query Atom Type
 *
 * @return Type of atom
 */
atom::type_t atom::get_type()
{
  return data_type;
}

/**
 * \brief Query Encoded Header Size
 *
 * @return Size of encoded header
 */
size_t atom::get_header_size()
{
  return head_bytes;
}
    
/**
 * \brief Get Unsigned Integer Value
 */
uint64_t atom::get_uint()
{
  // Sanity check
  if (data_type != atom::UINT)
  {
    throw topaz::exception("Atom is not unsigned integer");
  }
  
  // Pass it back
  return uint_val;
}

/**
 * \brief Get Signed Integer Value
 */
int64_t atom::get_int()
{
  // Sanity check
  if (data_type != atom::INT)
  {
    throw topaz::exception("Atom is not unsigned integer");
  }
  
  // Pass it back
  return int_val;
}

/**
 * \brief Get Binary Data
 */
byte_vector atom::get_bytes()
{
  // Sanity check
  if (data_type != atom::BYTES)
  {
    throw topaz::exception("Atom is not binary data");
  }
  
  // Copy data
  return byte_vector(begin() + head_bytes, end());
}

/**
 * \brief Decode internal storage
 */
void atom::decode()
{
  byte *data = &((*this)[0]);
  size_t count = 0;
  
  // Minimum 1 byte
  decode_check_size(1);
  
  // What is it?
  if (data[0] < atom::SHORT)
  {
    // Tiny Atom (Data stored in header)
    head_bytes = 0;
    
    // Determine type
    decode_set_type(0x03 & (data[0] >> 6));
    
    // Must be integer (Note: union type)
    int_val = 0x3f & data[0];
    if ((data_type == atom::INT) && (data[0] & 0x20))
    {
      // Negative signed integer - Sign extend
      int_val |= -1ULL & ~(0x3f);
    }
    
    // No further processing
    return;
  }
  else if (data[0] < atom::MEDIUM)
  {
    // Short Atom (1 byte header)
    head_bytes = 1;
    
    // Determine type
    decode_set_type(0x03 & (data[0] >> 4));
    
    // Determine size
    count = data[0] & 0x0f;
  }
  else if (data[0] < atom::LONG)
  {
    // Medium Atom (2 byte header)
    head_bytes = 2;
    decode_check_size(head_bytes);
    
    // Determine type
    decode_set_type(0x03 & (data[0] >> 3));
    
    // Determine size
    count = 0x07 & data[0];
    count = (count << 8) + data[1];
  }
  else if (data[0] < 0xf0)
  {
    // Long Atom (4 byte header)
    head_bytes = 4;
    decode_check_size(head_bytes);
    
    // Determine type
    decode_set_type(0x03 & data[0]);
    
    // Determine size
    count = data[1];
    count = (count << 8) + data[2];
    count = (count << 8) + data[3];
  }
  
  // Ensure expected remaining data is present
  decode_check_size(head_bytes + count);
  
  // Parse integers
  if (data_type != atom::BYTES)
  {
    decode_int();
  }
}

/**
 * \brief Ensure data to be decoded is of minimum size
 *
 * @param min Minimum size required for buffer
 */
void atom::decode_check_size(size_t min)
{
  if (size() < min)
  {
    throw topaz::exception("Atom encoding too short");
  }
}

/**
 * \brief Set atom type based on bin and sign flags
 *
 * @param bits Binary / Sign flags (values 0-3)
 */
void atom::decode_set_type(uint8_t bits)
{
  switch(bits)
  {
    case 0:
      data_type = atom::UINT;
      break;
      
    case 1:
      data_type = atom::INT;
      break;
      
    case 2:
      data_type = atom::BYTES;
      break;
      
    default:
      throw topaz::exception("Invalid / Unhandled atom type");
      break;
  }
}

/**
 * \brief Decode unsigned / signed integer
 */
void atom::decode_int()
{
  union
  {
    uint64_t flip;
    byte     raw[8];
  };
  
  // What we're decoding
  byte *data = &((*this)[0]);
  
  // How long is it
  size_t len = size() - head_bytes;
  
  // How many bytes don't get set in raw ...
  size_t skip = 8 - len;
  
  // Sanity check
  if ((len == 0) || (len > 8))
  {
    throw topaz::exception("Invalid integer Atom length");
  }
  
  // Set pointer to end of header
  data += head_bytes;
  
  // Sign extend negative values
  if ((data_type == atom::INT) && (data[0] & 0x80))
  {
    flip = -1;
  }
  else
  {
    flip = 0;
  }
  
  // Copy data over
  for (size_t i = 0; i < len; i++)
  {
    raw[skip + i] = data[i];
  }
  
  // Byteflip (Note: union type)
  uint_val = be64toh(flip);
}

/**
 * \brief Encode Atom Bytes
 *
 * @param data Pointer to bytes to encode
 * @param len  Number of bytes to encode
 */
void atom::encode_atom(void const *data, size_t len)
{
  byte next;
  
  // Fill out the header for the atom
  if (len < 16)
  {
    // Short atom (1 byte header)
    head_bytes = 1;
    
    // First byte
    next = atom::SHORT;
    if (data_type == atom::BYTES) next |= atom::SHORT_BIN;
    if (data_type == atom::INT) next |= atom::SHORT_SIGN;
    next |= len;
    push_back(next);
  }
  else if (len < 2048)
  {
    // Medium atom (2 byte header)
    head_bytes = 2;
    
    // First byte
    next = atom::MEDIUM;
    if (data_type == atom::BYTES) next |= atom::MEDIUM_BIN;
    if (data_type == atom::INT) next |= atom::MEDIUM_SIGN;
    next |= len >> 8;
    push_back(next);
    
    // Second byte
    push_back(0xff & len);
  }
  else if (len < 16777216)
  {
    // Long atom (4 byte header)
    head_bytes = 4;
    
    // First byte
    next = atom::LONG;
    if (data_type == atom::BYTES) next |= atom::LONG_BIN;
    if (data_type == atom::INT) next |= atom::LONG_SIGN;
    push_back(next);
    
    // Second byte
    next = 0xff & (len >> 16);
    push_back(next);
    
    // Third byte
    next = 0xff & (len >> 8);
    push_back(next);
    
    // Fourth byte
    next = 0xff & len;
    push_back(next);
  }
  else
  {
    // Really ???
    throw topaz::exception("Atom too large to encode");
  }
  
  // Append data bytes to end
  for (size_t i = 0; i < len; i++)
  {
    push_back(((byte*)data)[i]);
  }
}
