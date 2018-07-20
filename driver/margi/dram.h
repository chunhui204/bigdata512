/* 
    dram.h

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

#ifndef DRAM_H
#define DRAM_H

    /////////////////////////////////
   //                             //
  //  L64021 DRAM Memory Access  //
 //                             //
/////////////////////////////////

#include "cardbase.h"

   // where: 21 bit DRAM Word-Address, 4 word aligned
  // size: bytes (8 byte aligned, remainder will be filled with fill value)
 // data: fill value
// returns 0 on success, -1 on collision with DMA transfer
int DRAMFillByte(struct cvdv_cards *card, u32 where, int size, u8 data);

   // where: 21 bit DRAM Word-Address, 8 byte aligned
  // size: bytes (8 byte aligned, remainder will be filled with garbage)
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMWriteByte(struct cvdv_cards *card, u32 where, int size, u8 * data,
		  int swapburst);

   // where: 21 bit DRAM Word-Address, 4 word aligned
  // size: words (4 word aligned, remainder will be filled with garbage)
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMWriteWord(struct cvdv_cards *card, u32 where, int size, u16 * data,
		  int swap);

   // where: 21 bit DRAM Word-Address, 8 byte aligned
  // size: bytes
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMReadByte(struct cvdv_cards *card, u32 where, int size, u8 * data,
		 int swap);


   // where: 21 bit DRAM Word-Address, 4 word aligned
  // size: words
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMReadWord(struct cvdv_cards *card, u32 where, int size, u16 * data,
		 int swap);

     // where: 21 bit DRAM Word-Address, 4 word aligned
    // size: words
   // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
  // returns -1 on success (equal content), 
 //   word position on error (compare failure), 
//   -2 on collision with DMA transfer
int DRAMVerifyWord(struct cvdv_cards *card, u32 where, int size,
		   u16 * data, int swap);

      // WARNING: better not use this one. It can collide with normal DRAM access and other DMA transfers
     // If you want to use it, implement card->DMAMoveBusy in all other DMA functions, initialisation, and header file
    // source, destination: 21 bit DRAM Word-Address, 4 word aligned
   // size: byte (8 byte aligned, hang over bytes will NOT be moved)
  // returns 0 on success on success,
 //   -1 on collision with DMA transfer,
//   -2 on interrupt handler not installed
int DRAMMove(struct cvdv_cards *card, u32 source, u32 destination,
	     int size);

  // size in words
 // align:  number of words on wich start of block will be aligned
// return value is 21 bit word address, or 0xFFFFFFFF on error
u32 DRAMAlloc(struct cvdv_cards *card, u32 size, int align);

 // addr is the return value of that resp. DRAMAlloc call
// returns 0 on success (always)
int DRAMFree(struct cvdv_cards *card, u32 addr);

 // free all blocks
// returns 0 on success (always)
int DRAMRelease(struct cvdv_cards *card);

#endif				/* DRAM_H */
