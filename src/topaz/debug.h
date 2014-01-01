#ifndef TOPAZ_DEBUG_H
#define TOPAZ_DEBUG_H

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

/* Set to verbosity level (0 = none) */
extern int topaz_debug;

/* helper debug macro */
#define TOPAZ_DEBUG(x) if (topaz_debug >= (x))

/**
 * topaz_dump
 *
 * Dump binary data to console
 *
 * @param data Data buffer to display
 * @param len  Length of data buffer in bytes
 */
void topaz_dump(void const *data, int len);

#endif
