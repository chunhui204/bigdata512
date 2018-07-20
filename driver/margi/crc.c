/* 
    crc.c

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

Cyclic Redundancy Check 16 and 32 Bit

Christian Wolff, 19990122

---------------------------------------------------------*/

#define __NO_VERSION__

#include "crc.h"

unsigned short crc_16_table[256];
unsigned long crc_32_table[256];

// generate the tables of CRC-16 and CRC-32 remainders for all possible bytes
void gen_crc_table()
{
	register int i, j;
	register unsigned short crc16;
	register unsigned long crc32;
	for (i = 0; i < 256; i++) {
		crc16 = (unsigned short) i << 8;
		crc32 = (unsigned long) i << 24;
		for (j = 0; j < 8; j++) {
			if (crc16 & 0x8000)
				crc16 = (crc16 << 1) ^ POLYNOMIAL_16;
			else
				crc16 = (crc16 << 1);
			if (crc32 & 0x80000000L)
				crc32 = (crc32 << 1) ^ POLYNOMIAL_32;
			else
				crc32 = (crc32 << 1);
		}
		crc_16_table[i] = crc16;
		crc_32_table[i] = crc32;
	}
}

// update the CRC on the data block one byte at a time
unsigned short update_crc_16_block(unsigned short crc,
				   char *data_block_ptr,
				   int data_block_size)
{
	register int i;
	for (i = 0; i < data_block_size; i++)
		crc = update_crc_16(crc, *data_block_ptr++);
	return crc;
}

unsigned long update_crc_32_block(unsigned long crc, char *data_block_ptr,
				  int data_block_size)
{
	register int i;
	for (i = 0; i < data_block_size; i++)
		crc = update_crc_32(crc, *data_block_ptr++);
	return crc;
}
