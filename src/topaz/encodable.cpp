/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Encodable Object Interface Class
 *
 * This file an object class that can be serialized into a TCG Opal byte stream,
 * as well as deserialized into a copy of the original object.
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

#include <topaz/encodable.h>
#include <topaz/exceptions.h>
using namespace topaz;

/**
 * \brief Encodable Object Constructor
 */
encodable::encodable()
{
  // Nada
}

/**
 * \brief Encodable Object Destructor
 */
encodable::~encodable()
{
  // Nada
}

/**
 * \brief Encode to Container
 *
 * @return Encoded data
 */
byte_vector encodable::encode_vector() const
{
  byte_vector data;
  
  // Resize to appropriate size
  data.resize(size());
  
  // Set up the data call
  encode_bytes(&(data[0]));
  
  // All done
  return data;
}

/**
 * \brief Decode from Container
 *
 * @param data Specifed container
 * @return Number of bytes processed
 */
size_t encodable::decode_vector(byte_vector const &data)
{
  // Pass it on
  return decode_bytes(&(data[0]), data.size());
}
