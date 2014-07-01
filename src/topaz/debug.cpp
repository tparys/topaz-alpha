/**
 * File:   $URL $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Debug Definitions
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

#include <cstdio>
#include <topaz/debug.h>
using namespace topaz;

/* Set to verbosity level (0 = none) */
int topaz_debug = 0;

/**
 * topaz_dump
 *
 * Dump binary data to console
 *
 * @param data Data buffer to display
 * @param len  Length of data buffer in bytes
 */
void topaz::dump(void const *data, int len)
{
  unsigned char const *cdata = (unsigned char*)data;
  int i, j, chunk = 16;
  
  for (j = 0; j < len; j += chunk)
  {
    printf("%04x:", j);
    for (i = 0; (i < chunk) && (i + j < len); i++)
    {
      printf(" %02x", cdata[i + j]);
    }
    printf("\n");
  }
  
  printf("\n");
}
