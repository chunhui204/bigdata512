/* 
    crc.h

    Copyright (C) Christian Wolff for convergence integrated media.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*---------------------------------------------------------

Cyclic Redundancy Check 16 and 32 Bit - Header File

Christian Wolff, 19990122

---------------------------------------------------------*/

#ifndef _CRC_H_
#define _CRC_H_

// 16 Bit CCITT standard polynomial x^16 + x^12 + x^5 + x^1 + x^0
#define POLYNOMIAL_16 0x1021
// 32 Bit standard polynomial x^32 + x^26 + x^23 + x^22 + x^16 +
//   x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + x^0
#define POLYNOMIAL_32 0x04C11DB7L

#define CRC_INIT_16   0xFFFF
#define CRC_INIT_32   0xFFFFFFFFL

#define update_crc_16(crc, data) ((crc << 8) ^ crc_16_table[((int)(crc >> 8 ) ^ (data)) & 0xFF])
#define update_crc_32(crc, data) ((crc << 8) ^ crc_32_table[((int)(crc >> 24) ^ (data)) & 0xFF])

extern unsigned short crc_16_table[256];
extern unsigned long crc_32_table[256];

extern void gen_crc_table(void);

extern unsigned short update_crc_16_block(unsigned short crc,
					  char *data_block_ptr,
					  int data_block_size);
extern unsigned long update_crc_32_block(unsigned long crc,
					 char *data_block_ptr,
					 int data_block_size);

#endif
