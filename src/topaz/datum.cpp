/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Datum
 *
 * This class implements a TCG Opal Data Item, that is a higher level, possibly
 * aggregate data type. This includes basic atom types (integers and binary),
 * but also more complex aggregate types such as named types (key/value), lists,
 * and method calls.
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

#include <topaz/datum.h>
#include <topaz/exceptions.h>
using namespace topaz;

/**
 * \brief Default Constructor
 */
datum::datum()
{
  // Datum is empty atom
  data_type = datum::ATOM;
}

/**
 * \brief Atom Constructor
 */
datum::datum(atom const &value)
  : value(value)
{
  // Datum is specified atom
  data_type = datum::ATOM;
}

/**
 * \brief Named Constructor
 */
datum::datum(atom const &name, atom const &value)
  : name(name), value(value)
{
  // Named data type
  data_type = datum::NAMED;
}

/**
 * \brief List Constructor
 */
datum::datum(datum_vector const &list)
  : list(list)
{
  // List of other datums
  data_type = datum::LIST;
}

/**
 * \brief Method Constructor
 */
datum::datum(uint64_t object_uid, uint64_t method_uid, datum_vector const &args)
  : object_uid(object_uid), method_uid(method_uid), list(args)
{
  // Method call
  data_type = datum::METHOD;
}

/**
 * \brief Destructor
 */
datum::~datum()
{
  // Nada
}

/**
 * \brief Equality Operator
 *
 * @return True when equal
 */
bool datum::operator==(datum const &ref)
{
  // Check to make sure both are same data type
  if (data_type == ref.data_type)
  {
    // Type specific checks
    switch (data_type)
    {
      case datum::ATOM:
	// Compare atoms
	return value == ref.value;
	break;
	
      case datum::NAMED:
	// Compare name and value atoms
	return ((name == ref.name) && (value == ref.value));
	break;
	
      case datum::METHOD:
	// Compare method calls
	if ((object_uid != ref.object_uid) || (method_uid != ref.method_uid))
	{
	  // Failure
	  return false;
	}
	
	// No break - fall through to check list tokens (might be cheating)
	
      case datum::LIST:
	// Compare list items
	if (list.size() == ref.list.size())
	{
	  for (size_t i = 0; i < list.size(); i++)
	  {
	    if (list[i] != ref.list[i])
	    {
	      // Mismatch
	      return false;
	    }
	  }
	  
	  // All items match
	  return true;
	}
	break;
	
      default: // datum::END_SESSION
	return true;
	break;
    }
  }
  // Match fail
  return false;
}

/**
 * \brief Inequality Operator
 *
 * @return True when not equal
 */
bool datum::operator!=(datum const &ref)
{
  return !((*this) == ref);
}

/**
 * \brief Query encoded size
 *
 * @return Byte count of object when encoded
 */
size_t datum::size() const
{
  size_t i, count = 0;
  
  // Size varies based on contents
  switch (data_type)
  {
    case datum::ATOM:
      // Single Atom
      count += value.size();
      break;
      
    case datum::NAMED:
      // Key/Value Pair
      count += 2; // Token overhead
      count += name.size();
      count += value.size();
      break;
      
    case datum::LIST:
      // List 'o Things ...
      count += 2; // Token overhead
      for (i = 0; i < list.size(); i++)
      {
	count += list[i].size();
      }
      break;
      
    case datum::METHOD:
      // Method call
      count += 27; // Token overhead
      for (i = 0; i < list.size(); i++)
      {
	count += list[i].size();
      }
      break;
      
    default: // datum::END_SESSION
      // Single byte
      count += 1;
      break;
      
  }
  
  // All done
  return count;
}

/**
 * \brief Encode to data buffer
 *
 * @param data Data buffer of at least size() bytes
 * @return Number of bytes encoded
 */
size_t datum::encode_bytes(byte *data) const
{
  byte *orig = data;
  
  // Encoding varies based on type
  switch (data_type)
  {
    case datum::ATOM: // Single atom
      data += value.encode_bytes(data);
      break;
      
    case datum::NAMED: // Named key/value
      // Beginning of name
      *data++ = datum::TOK_START_NAME;
      
      // First value (key/name)
      data += name.encode_bytes(data);
      
      // Second value (named value)
      data += value.encode_bytes(data);
      
      // End of name
      *data++ = datum::TOK_END_NAME;
      break;
      
    case datum::LIST: // List 'o things
      // Beginning of list
      *data++ = datum::TOK_START_LIST;
      
      // Each item in the list
      for (size_t i = 0; i < list.size(); i++)
      {
	data += list[i].encode_bytes(data);
      }
      
      // End of list
      *data++ = datum::TOK_END_LIST;
      break;
      
    case datum::METHOD: // Method call
      // Beginning of method call
      *data++ = datum::TOK_CALL;
      
      // Object UID
      data += atom(object_uid, true).encode_bytes(data);
      
      // Method UID
      data += atom(method_uid, true).encode_bytes(data);
      
      // Beginning of parameter list (arguments)
      *data++ = datum::TOK_START_LIST;
      
      // Each item in the list
      for (size_t i = 0; i < list.size(); i++)
      {
	data += list[i].encode_bytes(data);
      }
      
      // End of parameter list (arguments)
      *data++ = datum::TOK_END_LIST;
      
      // End of data
      *data++ = datum::TOK_END_OF_DATA;
      
      // Beginning of method status list
      *data++ = datum::TOK_START_LIST;
      
      // Abort method?
      *data++ = 0x00; // No
      
      // Reserved fields
      *data++ = 0x00;
      *data++ = 0x00;
      
      // End of method status list
      *data++ = datum::TOK_END_LIST;
      break;
      
    default: // datum::END_SESSION
      // Single byte
      *data++ = datum::TOK_END_SESSION;
      break;
  }
  
  return data - orig;
}

/**
 * \brief Decode from data buffer
 *
 * @param data Location to read encoded bytes
 * @param len  Length of buffer
 * @return Number of bytes processed
 */
size_t datum::decode_bytes(byte const *data, size_t len)
{
  size_t size = 0;
  
  // Minimum 1 byte
  decode_check_size(len, 1);
  
  // What is it?
  if (data[size] == datum::TOK_START_LIST)
  {
    // Start of a list type
    data_type = datum::LIST;
    size++;
    
    // Loop until we hit the end of the list
    while (1)
    {
      // Ensure one more byte
      decode_check_size(len, size + 1);
      
      // End of list?
      if (data[size] == datum::TOK_END_LIST)
      {
	// Done here
	size++;
	break;
      }
      
      // Else, assume some other datum type
      topaz::datum tmp;
      size += tmp.decode_bytes(data + size, len - size);
      list.push_back(tmp);
    }
  }
  else if (data[size] == datum::TOK_START_NAME)
  {
    // Named data type
    data_type = datum::NAMED;
    size++;
    
    // Name
    size += name.decode_bytes(data + size, len - size);
    
    // Value
    size += value.decode_bytes(data + size, len - size);
    
    // End of named type
    decode_check_token(data, len, size++, datum::TOK_END_NAME);
  }
  else if (data[size] == datum::TOK_CALL)
  {
    topaz::atom uid;
    
    // Method call
    data_type = datum::METHOD;
    size++;
    
    // Object UID
    size += uid.decode_bytes(data + size, len - size);
    object_uid = uid.get_uid();

    // Method UID
    size += uid.decode_bytes(data + size, len - size);
    method_uid = uid.get_uid();
    
    // Beginning of parameter list (arguments
    decode_check_token(data, len, size++, datum::TOK_START_LIST);
    
    // Loop until we hit the end of the list
    while (1)
    {
      // Ensure one more byte
      decode_check_size(len, size + 1);
      
      // End of list?
      if (data[size] == datum::TOK_END_LIST)
      {
	// Done here
	size++;
	break;
      }
      
      // Else, assume some other datum type
      topaz::datum tmp;
      size += tmp.decode_bytes(data + size, len - size);
      list.push_back(tmp);
    }
    
    // End of data
    decode_check_token(data, len, size++, datum::TOK_END_OF_DATA);
    
    // Method status list
    decode_check_token(data, len, size++, datum::TOK_START_LIST);
    decode_check_token(data, len, size++, 0x00);
    decode_check_token(data, len, size++, 0x00);
    decode_check_token(data, len, size++, 0x00);
    decode_check_token(data, len, size++, datum::TOK_END_LIST);
  }
  else if (data[size] == datum::END_SESSION)
  {
    // End of Session indicator
    data_type = datum::END_SESSION;
    size++;
  }
  else
  {
    // Failing that, assume it's an atom
    data_type = datum::ATOM;
    size += value.decode_bytes(data + size, len - size);
  }
  
  return size;
}

/**
 * \brief Query Datum Type
 *
 * @return Type of datum
 */
datum::type_t datum::get_type()
{
  return data_type;
}

/**
 * \brief Ensure data to be decoded is of minimum size
 *
 * @param len Length of buffer
 * @param min Minimum size required for buffer
 */
void datum::decode_check_size(size_t len, size_t min) const
{
  if (len < min)
  {
    throw topaz::exception("Datum encoding too short");
  }
}

/**
 * \brief Verify next token exists and is expected value
 *
 * @param data Location to read encoded bytes
 * @param len  Length of buffer
 * @param idx  Next index
 * @param next Expected value at index
 */
void datum::decode_check_token(byte const *data, size_t len, size_t idx, byte next) const
{
  // Next token doesn't exist
  if (idx >= len)
  {
    throw topaz::exception("Datum encoding too short");
  }
  if (data[idx] != next)
  {
    throw topaz::exception("Unexpected token in datum encoding");
  }
}
