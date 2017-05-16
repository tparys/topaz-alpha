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
