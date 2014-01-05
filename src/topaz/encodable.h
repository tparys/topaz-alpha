#ifndef TOPAZ_ENCODABLE_H
#define TOPAZ_ENCODABLE_H

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

#include <topaz/defs.h>

namespace topaz
{

  class encodable
  {
    
  public:
    
    // Constructor / Destructor
    encodable();
    virtual ~encodable();
    
    /**
     * \brief Query encoded size
     *
     * @return Byte count of object when encoded
     */
    virtual size_t size() const = 0;
    
    /**
     * \brief Encode to data buffer
     *
     * @param data Data buffer of at least size() bytes
     * @return Number of bytes encoded
     */
    virtual size_t encode_bytes(byte *data) const = 0;
    
    /**
     * \brief Encode to Container
     *
     * @return Encoded data
     */
    byte_vector encode_vector() const;
    
    /**
     * \brief Decode from data buffer
     *
     * @param data Location to read encoded bytes
     * @param len  Length of buffer
     * @return Number of bytes processed
     */
    virtual size_t decode_bytes(byte const *data, size_t len) = 0;
    
    /**
     * \brief Decode from Container
     *
     * @param data Specifed container
     */
    void decode_vector(byte_vector const &data);
    
    /**
     * \brief Debug print
     */
    virtual void print() const = 0;
    
  };
  
};

#endif
