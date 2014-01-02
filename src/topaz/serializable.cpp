/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Serializable Object Interface Class
 *
 * This file an object class that can be serialized into a given byte stream,
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

#include <topaz/serializable.h>
using namespace topaz;

/**
 * \brief Serializable Object Constructor
 */
serializable::serializable()
{
  // Nada
}

/**
 * \brief Serializable Object Destructor
 */
serializable::~serializable()
{
  // Nada
}

/**
 * \brief Decode from data stream
 *
 * @param data Location to read encoded bytes
 * @param len  Length of buffer
 */
void serializable::deserialize(void const *data, size_t len)
{
  // Load data
  resize(len);
  for (size_t i = 0; i < len; i++)
  {
    (*this)[i] = ((byte*)data)[i];
  }
  
  // Parse data
  decode();
}

/**
 * \brief Decode from data container
 *
 * @param data Container to read encoded bytes
 */
void serializable::deserialize(byte_vector const &data)
{
  // Load data
  resize(data.size());
  for (size_t i = 0; i < data.size(); i++)
  {
    (*this)[i] = data[i];
  }
  
  // Parse data
  decode();
}
