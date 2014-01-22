/**
 * File:   $URL $
 * Author: $Author $
 * Date:   $Date $
 * Rev:    $Revision $
 *
 * Topaz - Debug Definitions
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
