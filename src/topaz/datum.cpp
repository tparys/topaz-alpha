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

#include <cstdio>
#include <topaz/datum.h>
#include <topaz/exceptions.h>
using namespace topaz;

/**
 * \brief Default Constructor
 */
datum::datum()
{
  // Datum type not yet known
  data_type = datum::UNSET;
  data_object_uid = 0;
  data_method_uid = 0;
  data_status = STA_SUCCESS;
}

/**
 * \brief Token Constructor
 */
datum::datum(datum::type_t data_type)
  : data_type(data_type)
{
  // Specified datum type
  data_object_uid = 0;
  data_method_uid = 0;
  data_status = STA_SUCCESS;
  
  // Make room for named value, if needed
  if (data_type == datum::NAMED)
  {
    data_list.resize(1);
  }
}

/**
 * \brief Atom->Datum Promotion Constructor
 */
datum::datum(atom val)
{
  // Datum type not yet known
  data_type = datum::ATOM;
  data_atom = val;
  data_object_uid = 0;
  data_method_uid = 0;
  data_status = STA_SUCCESS;
}

/**
 * \brief Destructor
 */
datum::~datum()
{
  // Nada
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
      count += data_atom.size();
      break;
      
    case datum::NAMED:
      // Key/Value Pair
      count += 2; // Token overhead
      count += data_atom.size();
      count += data_list[0].size();
      break;
      
    case datum::LIST:
      // List 'o Things ...
      count += 2; // Token overhead
      for (i = 0; i < data_list.size(); i++)
      {
	count += data_list[i].size();
      }
      break;
      
    case datum::METHOD:
      // Method call
      count += 27; // Token overhead
      for (i = 0; i < data_list.size(); i++)
      {
	count += data_list[i].size();
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
      data += data_atom.encode_bytes(data);
      break;
      
    case datum::NAMED: // Named key/value
      // Beginning of name
      *data++ = datum::TOK_START_NAME;
      
      // First value (key/name)
      data += data_atom.encode_bytes(data);
      
      // Second value (named value)
      data += data_list[0].encode_bytes(data);
      
      // End of name
      *data++ = datum::TOK_END_NAME;
      break;
      
    case datum::LIST: // List 'o things
      // Beginning of list
      *data++ = datum::TOK_START_LIST;
      
      // Each item in the list
      for (size_t i = 0; i < data_list.size(); i++)
      {
	data += data_list[i].encode_bytes(data);
      }
      
      // End of list
      *data++ = datum::TOK_END_LIST;
      break;
      
    case datum::METHOD: // Method call
      // Beginning of method call
      *data++ = datum::TOK_CALL;
      
      // Object UID
      data += atom::new_uid(data_object_uid).encode_bytes(data);
      
      // Method UID
      data += atom::new_uid(data_method_uid).encode_bytes(data);
      
      // Beginning of parameter list (arguments)
      *data++ = datum::TOK_START_LIST;
      
      // Each item in the list
      for (size_t i = 0; i < data_list.size(); i++)
      {
	data += data_list[i].encode_bytes(data);
      }
      
      // End of parameter list (arguments)
      *data++ = datum::TOK_END_LIST;
      
      // End of data
      *data++ = datum::TOK_END_OF_DATA;
      
      // Beginning of method status list
      *data++ = datum::TOK_START_LIST;
      
      // Method status
      *data++ = 0xff & data_status;
      
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
  
  // Clear out any stale data
  data_list.clear();
  
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
      datum tmp;
      size += tmp.decode_bytes(data + size, len - size);
      data_list.push_back(tmp);
    }
  }
  else if (data[size] == datum::TOK_START_NAME)
  {
    // Named data type
    data_type = datum::NAMED;
    size++;
    
    // Name
    size += data_atom.decode_bytes(data + size, len - size);
    
    // Value
    data_list.resize(1);
    size += data_list[0].decode_bytes(data + size, len - size);
    
    // End of named type
    decode_check_token(data, len, size++, datum::TOK_END_NAME);
  }
  else if (data[size] == datum::TOK_CALL)
  {
    atom uid;
    
    // Method call
    data_type = datum::METHOD;
    size++;
    
    // Object UID
    size += uid.decode_bytes(data + size, len - size);
    data_object_uid = uid.get_uid();

    // Method UID
    size += uid.decode_bytes(data + size, len - size);
    data_method_uid = uid.get_uid();
    
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
      datum tmp;
      size += tmp.decode_bytes(data + size, len - size);
      data_list.push_back(tmp);
    }
    
    // End of data
    decode_check_token(data, len, size++, datum::TOK_END_OF_DATA);
    
    // Method status list - 5 bytes
    decode_check_size(len, size + 5);
    
    // First and last token are required
    if ((data[size] != datum::TOK_START_LIST) ||
	(data[size + 4] != datum::TOK_END_LIST))
    {
      throw topaz_exception("Unexpected token in datum encoding");
    }
    
    // Second byte has method status
    data_status = (status_t)data[size + 1];
    
    // 5 Bytes total
    size += 5;
  }
  else if (data[size] == datum::TOK_END_SESSION)
  {
    // End of Session indicator
    data_type = datum::END_SESSION;
    size++;
  }
  else
  {
    // Failing that, assume it's an atom
    data_type = datum::ATOM;
    size += data_atom.decode_bytes(data + size, len - size);
  }
  
  return size;
}

/**
 * \brief Query Datum Type
 */
datum::type_t datum::get_type() const
{
  return data_type;
}

/**
 * \brief Query Atom Value
 */
atom &datum::value()
{
  // Must be atom or named type
  if (data_type == datum::UNSET)
  {
    // Automatic promotion
    data_type = datum::ATOM;
  }
  else if (data_type != datum::ATOM)
  {
    throw topaz_exception("Datum has no value");
  }
  
  // Return
  return data_atom;
}

/**
 * \brief Query Atom Value (const)
 */
atom const &datum::value() const
{
  // Must be atom or named type
  if (data_type != datum::ATOM)
  {
    throw topaz_exception("Datum has no value");
  }
  
  // Return
  return data_atom;
}

/**
 * \brief Query Name
 */
atom &datum::name()
{
  // Must be named type
  if (data_type == datum::UNSET)
  {
    // Automatic promotion
    data_type = datum::NAMED;
    data_list.resize(1);
  }
  else if (data_type != datum::NAMED)
  {
    throw topaz_exception("Datum has no name");
  }
  
  // Return
  return data_atom;
}

/**
 * \brief Query Name (const)
 */
atom const &datum::name() const
{
  // Must be named type
  if (data_type != datum::NAMED)
  {
    throw topaz_exception("Datum has no name");
  }
  
  // Return
  return data_atom;
}

/**
 * \brief Query Named Value
 */
datum &datum::named_value()
{
  // Must be named type
  if (data_type == datum::UNSET)
  {
    // Automatic promotion
    data_type = datum::NAMED;
    data_list.resize(1);
  }
  else if (data_type != datum::NAMED)
  {
    throw topaz_exception("Datum has no named value");
  }
  
  // Return
  return data_list[0];
}

/**
 * \brief Query Named Value (const)
 */
datum const &datum::named_value() const
{
  // Must be named type
  if (data_type != datum::NAMED)
  {
    throw topaz_exception("Datum has no named value");
  }
  
  // Return
  return data_list[0];
}

/**
 * \brief Query Method's Object UID
 */
uint64_t &datum::object_uid()
{
  // Must be method
  if ((data_type == datum::UNSET) || (data_type == datum::LIST))
  {
    // Automatic promotion
    data_type = datum::METHOD;
  }
  else if (data_type != datum::METHOD)
  {
    throw topaz_exception("Datum has no object UID");
  }
  
  return data_object_uid;
}

/**
 * \brief Query Method's Object UID (const)
 */
uint64_t const &datum::object_uid() const
{
  // Must be method
  if (data_type != datum::METHOD)
  {
    throw topaz_exception("Datum has no object UID");
  }
  
  return data_object_uid;
}

/**
 * \brief Query Value Method UID
 */
uint64_t &datum::method_uid()
{
  // Must be method
  if ((data_type == datum::UNSET) || (data_type == datum::LIST))
  {
    // Automatic promotion
    data_type = datum::METHOD;
  }
  else if (data_type != datum::METHOD)
  {
    throw topaz_exception("Datum has no method UID");
  }
  
  return data_method_uid;
}

/**
 * \brief Query Value Method UID (const)
 */
uint64_t const &datum::method_uid() const
{
  // Must be method
  if (data_type != datum::METHOD)
  {
    throw topaz_exception("Datum has no method UID");
  }
  
  return data_method_uid;
}

/**
 * \brief Query Method Status Code
 */
datum::status_t &datum::status()
{
  // Must be method
  if ((data_type == datum::UNSET) || (data_type == datum::LIST))
  {
    // Automatic promotion
    data_type = datum::METHOD;
  }
  else if (data_type != datum::METHOD)
  {
    throw topaz_exception("Datum has no status");
  }
  
  return data_status;
}

/**
 * \brief Query Method Status Code (const)
 */
datum::status_t const &datum::status() const
{
  // Must be method
  if (data_type != datum::METHOD)
  {
    throw topaz_exception("Datum has no status");
  }
  
  return data_status;
}

/**
 * \brief Query List
 */
datum_vector &datum::list()
{
  // Must be list or method call
  if (data_type == datum::UNSET)
  {
    // Automatic promotion
    data_type = datum::LIST;
  }
  else if ((data_type != datum::LIST) && (data_type != datum::METHOD))
  {
    throw topaz_exception("Datum has no list");
  }
  
  // Return
  return data_list;
}

/**
 * \brief Query List (const)
 */
datum_vector const &datum::list() const
{
  // Must be list or method call
  if ((data_type != datum::LIST) && (data_type != datum::METHOD))
  {
    throw topaz_exception("Datum has no list");
  }
  
  // Return
  return data_list;
}

/**
 * \brief Query Named Value in List
 */
datum &datum::find_by_name(uint64_t id)
{
  // Must be list
  if (data_type != datum::LIST)
  {
    throw topaz_exception("Datum has no list");
  }
  
  // Search for named value
  for (size_t i = 0; i < data_list.size(); i++)
  {
    if ((data_list[i].get_type() == datum::NAMED) &&
	(data_list[i].name().get_uint() == id))
    {
      return data_list[i].named_value();
    }
  }
  
  throw topaz_exception("Named value not found in list");
}

/**
 * \brief Query Named Value in List (const)
 */
datum const &datum::find_by_name(uint64_t id) const
{
  // Must be list
  if (data_type != datum::LIST)
  {
    throw topaz_exception("Datum has no list");
  }
  
  // Search for named value
  for (size_t i = 0; i < data_list.size(); i++)
  {
    if ((data_list[i].get_type() == datum::NAMED) &&
	(data_list[i].name().get_uint() == id))
    {
      return data_list[i].named_value();
    }
  }
  
  throw topaz_exception("Named value not found in list");
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
	return data_atom == ref.data_atom;
	break;
	
      case datum::NAMED:
	// Compare name and value atoms
	return ((data_atom == ref.data_atom) &&
		(data_list[0] == ref.data_list[0]));
	break;
	
      case datum::METHOD:
	// Compare method calls
	if ((data_object_uid != ref.data_object_uid) ||
	    (data_method_uid != ref.data_method_uid))
	{
	  // Failure
	  return false;
	}
	
	// No break here - fall through to check list tokens
	
      case datum::LIST:
	// Compare list items
	if (data_list.size() == ref.data_list.size())
	{
	  for (size_t i = 0; i < data_list.size(); i++)
	  {
	    if (data_list[i] != ref.data_list[i])
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
 * \brief Datum array access
 *
 * @param idx Item to return
 * @return Specified item
 */
datum &datum::operator[](size_t idx)
{
  // Must be list or method call
  if (data_type == datum::UNSET)
  {
    data_type = datum::LIST;
  }
  else if ((data_type != datum::LIST) && (data_type != datum::METHOD))
  {
    throw topaz_exception("Datum has no list");
  }
  
  // Resize if too small
  if (data_list.size() <= idx)
  {
    data_list.resize(idx + 1);
  }
  
  // Return item
  return data_list[idx];
}

/**
 * \brief Debug print
 */
void datum::print() const
{
  size_t i;
  
  // Determine what it is
  switch (data_type)
  {
    case datum::UNSET:
      // Nothing yet
      printf("(UNSET)");
      break;
      
    case datum::ATOM:
      // Single atom
      data_atom.print();
      break;
      
    case datum::NAMED:
      // Named value
      data_atom.print();
      printf(" = ");
      data_list[0].print();
      break;
           
    case datum::LIST:
      // List o' things
      printf("[");
      for (i = 0; i < data_list.size(); i++)
      {
	if (i > 0) printf(", ");
	data_list[i].print();
      }
      printf("]");
      break;
      
    case datum::METHOD:
      // Method Call
      atom::new_uid(data_object_uid).print();
      printf(".");
      atom::new_uid(data_method_uid).print();
      printf("[");
      for (i = 0; i < data_list.size(); i++)
      {
	if (i > 0) printf(", ");
	data_list[i].print();
      }
      printf("]");
      if (data_status)
      {
	printf("<STATUS=0x%x>", data_status);
      }
      break;
      
      // No break, fall through to list
    default: // datum::END_SESSION
      // End of session
      printf("(END SESSION)");
      break;
  }
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
    throw topaz_exception("Datum encoding too short");
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
    throw topaz_exception("Datum encoding too short");
  }
  if (data[idx] != next)
  {
    throw topaz_exception("Unexpected token in datum encoding");
  }
}
