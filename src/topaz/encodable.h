#ifndef TOPAZ_ENCODABLE_H
#define TOPAZ_ENCODABLE_H

/**
 * Topaz - Encodable Object Interface Class
 *
 * This file an object class that can be serialized into a TCG Opal byte stream,
 * as well as deserialized into a copy of the original object.
 *
 * Copyright (c) 2014, T Parys
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
         * @return Number of bytes processed
         */
        size_t decode_vector(byte_vector const &data);

        /**
         * \brief Debug print
         */
        virtual void print() const = 0;

    };

};

#endif
