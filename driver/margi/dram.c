/* 
    dram.c

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

    /////////////////////////////////
   //                             //
  //  L64021 DRAM Memory Access  //
 //                             //
/////////////////////////////////

#define __NO_VERSION__

#include "dram.h"
#include "l64021.h"

#define EMERGENCYCOUNTER 5

   // where: 21 bit DRAM Word-Address, 4 word aligned
  // size: bytes (8 byte aligned, remainder will be filled with fill value)
 // data: fill value
// returns 0 on success, -1 on collision with DMA transfer
int DRAMFillByte(struct cvdv_cards *card, u32 where, int size, u8 data)
{
	int i, j, k, n;
	u8 volatile flag;

	size = (size >> 3) + ((size & 7) ? 1 : 0);	// 8 bytes at a time, padding with garbage
	where >>= 2;		// 8 byte aligned data
	DecoderSetByte(card, 0x0C1, 0x08);
//TODO: 0x80?

	DecoderWriteByte(card, 0x0C6, (u8) ((where >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0C5, (u8) ((where >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0C4, (u8) (where & 0x000000FFL));
	i = 0;
	for (j = 0; j < size; j++) {
		for (k = 0; k < 8; k++) {
			n = EMERGENCYCOUNTER;
			do {	// wait if FIFO full
				flag = DecoderReadByte(card, 0x0C0);
			} while ((flag & 0x08) && n--);
			if (n<0)
				return -1;
			DecoderWriteByte(card, 0x0C3, data);
		}
	}
	flag = DecoderReadByte(card, 0x0C0);
	n = EMERGENCYCOUNTER;
	do {			// wait for FIFO empty
		flag = DecoderReadByte(card, 0x0C0);
	} while (!(flag & 0x04) && n--);
	return ((n>=0) ? 0 : -1);
}

   // where: 21 bit DRAM Word-Address, 8 byte aligned
  // size: bytes (8 byte aligned, remainder will be filled with garbage)
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMWriteByte(struct cvdv_cards *card, u32 where, int size, u8 * data,
		  int swapburst)
{
	int i, j, k, n;
	u8 volatile flag;

	size = (size >> 3) + ((size & 7) ? 1 : 0);	// 8 bytes at a time, padding with garbage
	where >>= 2;		// 8 byte aligned data
	MDEBUG(4, ": Moving %d 64-bit-words to DRAM 0x%08X\n",size,where);
	//if (swap) DecoderDelByte(card,0x0C1,0x08);  // byte swapping of 8 byte bursts
	//else      DecoderSetByte(card,0x0C1,0x08);  // no byte swapping
	DecoderSetByte(card, 0x0C1, 0x08);	// no byte swapping

	DecoderWriteByte(card, 0x0C6, (u8) ((where >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0C5, (u8) ((where >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0C4, (u8) (where & 0x000000FFL));
	i = 0;
	if (swapburst) {
		for (j = 0; j < size; j++) {
			for (k = 7; k >= 0; k--) {
				n = EMERGENCYCOUNTER;
				do {	// wait if FIFO full
					flag =
					    DecoderReadByte(card, 0x0C0);
				} while ((flag & 0x08) && n--);
				if (n<0)
					return -1;
				DecoderWriteByte(card, 0x0C3, data[i + k]);
			}
			i += 8;
		}
	} else {
		for (j = 0; j < size; j++) {
			for (k = 0; k < 8; k++) {
				n = EMERGENCYCOUNTER;
				do {	// wait if FIFO full
					flag =
					    DecoderReadByte(card, 0x0C0);
				} while ((flag & 0x08) && n--);
				if (n<0)
					return -1;
				DecoderWriteByte(card, 0x0C3, data[i++]);
			}
		}
	}
	flag = DecoderReadByte(card, 0x0C0);
	n = EMERGENCYCOUNTER;
	do {			// wait for FIFO empty
		flag = DecoderReadByte(card, 0x0C0);
	} while (!(flag & 0x04) && n--);
	return ((n>=0) ? 0 : -1);
}

   // where: 21 bit DRAM Word-Address, 4 word aligned
  // size: words (4 word aligned, remainder will be filled with garbage)
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMWriteWord(struct cvdv_cards *card, u32 where, int size, u16 * data,
		  int swap)
{
	int i, j, k, n;
	u8 volatile flag;

	size = (size >> 2) + ((size & 3) ? 1 : 0);	// 4 words at a time, padding with garbage
	where >>= 2;		// 8 byte aligned data
	MDEBUG(4, ": Moving %d 64-bit-words to DRAM 0x%08X\n",size,where);
//TODO: swap manually
	if (swap)
		DecoderDelByte(card, 0x0C1, 0x08);	// byte swapping of 8 byte bursts
	else
		DecoderSetByte(card, 0x0C1, 0x08);	// no byte swapping

	DecoderWriteByte(card, 0x0C6, (u8) ((where >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0C5, (u8) ((where >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0C4, (u8) (where & 0x000000FFL));
	i = 0;
	for (j = 0; j < size; j++) {
		for (k = 0; k < 4; k++) {
			n = EMERGENCYCOUNTER;
			do {	// wait if FIFO full
				flag = DecoderReadByte(card, 0x0C0);
			} while ((flag & 0x08) && n--);
			if (n<0)
				return -1;
			DecoderWriteByte(card, 0x0C3, data[i] >> 8);
			n = EMERGENCYCOUNTER;
			do {	// wait if FIFO full
				flag = DecoderReadByte(card, 0x0C0);
			} while ((flag & 0x08) && n--);
			if (n<0)
				return -1;
			DecoderWriteByte(card, 0x0C3, data[i++]);
		}
	}
	flag = DecoderReadByte(card, 0x0C0);
	n = EMERGENCYCOUNTER;
	do {			// wait for FIFO empty
		flag = DecoderReadByte(card, 0x0C0);
	} while (!(flag & 0x04) && n--);
	return ((n>=0) ? 0 : -1);
}

   // where: 21 bit DRAM Word-Address, 8 byte aligned
  // size: bytes
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMReadByte(struct cvdv_cards *card, u32 where, int size, u8 * data,
		 int swap)
{
	int i, j, rsize, n;
	u8 volatile flag;

	rsize = size & 7;	// padding bytes
	size = size >> 3;	// 8 bytes at a time
	where >>= 2;		// 8 byte aligned data
	MDEBUG(4, ": Moving %d 64-bit-words to DRAM 0x%08X\n",size,where);
//TODO: swap manually
	if (swap)
		DecoderDelByte(card, 0x0C1, 0x08);	// byte swapping of 8 byte bursts
	else
		DecoderSetByte(card, 0x0C1, 0x08);	// no byte swapping

	DecoderWriteByte(card, 0x0C9, (u8) ((where >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0C8, (u8) ((where >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0C7, (u8) (where & 0x000000FFL));
	i = 0;
	for (j = 0; j < size; j++) {
		n = EMERGENCYCOUNTER;
		do {		// wait if FIFO empty
			flag = DecoderReadByte(card, 0x0C0);
		} while ((flag & 0x01) && n--);
		if (n<0)  // WARNING nicht if(!n)
			return -1;
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
		data[i++] = DecoderReadByte(card, 0x0C2);
	}
	n = EMERGENCYCOUNTER;
	do {			// wait if FIFO empty
		flag = DecoderReadByte(card, 0x0C0);
	} while ((flag & 0x01) && n--);
	if (n<0)
		return -1;
	for (j = 0; j < rsize; j++)
		data[i++] = DecoderReadByte(card, 0x0C2);
	flag = DecoderReadByte(card, 0x0C0);
	n = EMERGENCYCOUNTER;
	do {			// wait for FIFO full
		flag = DecoderReadByte(card, 0x0C0);
	} while (!(flag & 0x02) && n--);
	return ((n>=0) ? 0 : -1);
}


   // where: 21 bit DRAM Word-Address, 4 word aligned
  // size: words
 // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
// returns 0 on success, -1 on collision with DMA transfer
int DRAMReadWord(struct cvdv_cards *card, u32 where, int size, u16 * data,
		 int swap)
{
	int i, j, rsize, n;
	u8 volatile flag;
	u8 b;

	rsize = size & 3;	// padding words
	size >>= 2;		// 4 words at a time
	where >>= 2;		// 8 byte aligned data
	MDEBUG(4, ": Reading %d 64-bit-words and %d 16-bit-words from DRAM 0x%08X\n",
	       size,rsize,where);
//TODO: swap manually
	if (swap)
		DecoderDelByte(card, 0x0C1, 0x08);	// byte swapping of 8 byte bursts
	else
		DecoderSetByte(card, 0x0C1, 0x08);	// no byte swapping

	DecoderWriteByte(card, 0x0C9, (u8) ((where >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0C8, (u8) ((where >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0C7, (u8) (where & 0x000000FFL));
	i = 0;
	for (j = 0; j < size; j++) {
		n = EMERGENCYCOUNTER;
		do {		// wait if FIFO empty
			flag = DecoderReadByte(card, 0x0C0);
		} while ((flag & 0x01) && n--);
		if (n<0)
			return -1;
		b = DecoderReadByte(card, 0x0C2);
		data[i++] = ((b << 8) | DecoderReadByte(card, 0x0C2));
		b = DecoderReadByte(card, 0x0C2);
		data[i++] = ((b << 8) | DecoderReadByte(card, 0x0C2));
		b = DecoderReadByte(card, 0x0C2);
		data[i++] = ((b << 8) | DecoderReadByte(card, 0x0C2));
		b = DecoderReadByte(card, 0x0C2);
		data[i++] = ((b << 8) | DecoderReadByte(card, 0x0C2));
	}
	n = EMERGENCYCOUNTER;
	do {			// wait if FIFO empty
		flag = DecoderReadByte(card, 0x0C0);
	} while ((flag & 0x01) && n--);
	if (n<0)
		return -1;
	for (j = 0; j < rsize; j++) {
		b = DecoderReadByte(card, 0x0C2);
		data[i++] = ((b << 8) | DecoderReadByte(card, 0x0C2));
	}
	flag = DecoderReadByte(card, 0x0C0);
	n = EMERGENCYCOUNTER;
	do {			// wait for FIFO full
		flag = DecoderReadByte(card, 0x0C0);
	} while (!(flag & 0x02) && n--);
	return ((n>=0) ? 0 : -1);
}

     // where: 21 bit DRAM Word-Address, 4 word aligned
    // size: words
   // swap: 0=normal mode, 1=write each 8 bytes on reverse order (7,6,5,4,3,2,1,0,15,14,13,etc.)
  // returns -1 on success (equal content), 
 //   word position on error (compare failure), 
//   -2 on collision with DMA transfer
int DRAMVerifyWord(struct cvdv_cards *card, u32 where, int size,
		   u16 * data, int swap)
{
	int i, j, rsize, n;
	u8 volatile flag, b;

	rsize = size & 3;	// padding words
	size >>= 2;		// 4 words at a time
	where >>= 2;		// 8 byte aligned data, now 19 bit 64-bit-word-address
//TODO: swap manually
	if (swap)
		DecoderDelByte(card, 0x0C1, 0x08);	// byte swapping of 8 byte bursts
	else
		DecoderSetByte(card, 0x0C1, 0x08);	// no byte swapping

	DecoderWriteByte(card, 0x0C9, (u8) ((where >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0C8, (u8) ((where >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0C7, (u8) (where & 0x000000FFL));
	i = 0;
	for (j = 0; j < size; j++) {
		n = EMERGENCYCOUNTER;
		do {		// wait if FIFO empty
			flag = DecoderReadByte(card, 0x0C0);
		} while ((flag & 0x01) && n--);
		b = DecoderReadByte(card, 0x0C2);
		if (data[i++] != ((b << 8) | DecoderReadByte(card, 0x0C2)))
			return i;
		b = DecoderReadByte(card, 0x0C2);
		if (data[i++] != ((b << 8) | DecoderReadByte(card, 0x0C2)))
			return i;
		b = DecoderReadByte(card, 0x0C2);
		if (data[i++] != ((b << 8) | DecoderReadByte(card, 0x0C2)))
			return i;
		b = DecoderReadByte(card, 0x0C2);
		if (data[i++] != ((b << 8) | DecoderReadByte(card, 0x0C2)))
			return i;
	}
	n = EMERGENCYCOUNTER;
	do {			// wait if FIFO empty
		flag = DecoderReadByte(card, 0x0C0);
	} while ((flag & 0x01) && n--);
	for (j = 0; j < rsize; j++) {
		b = DecoderReadByte(card, 0x0C2);
		if (data[i++] != ((b << 8) | DecoderReadByte(card, 0x0C2)))
			return i;
	}
	flag = DecoderReadByte(card, 0x0C0);
	n = EMERGENCYCOUNTER;
	do {			// wait for FIFO full
		flag = DecoderReadByte(card, 0x0C0);
	} while (!(flag & 0x02) && n--);
	return -1;
}

      // WARNING: better not use this one. It can collide with normal DRAM access and other DMA transfers
     // If you want to use it, implement card->DMAMoveBusy in all other DMA functions, initialisation, and header file
    // source, destination: 21 bit DRAM Word-Address, 4 word aligned
   // size: byte (8 byte aligned, hang over bytes will NOT be moved)
  // returns 0 on success on success,
 //   -1 on collision with DMA transfer,
//   -2 on interrupt handler not installed
int DRAMMove(struct cvdv_cards *card, u32 source, u32 destination,
	     int size)
{
	if (!card->IntInstalled)
		return -2;
	if (card->DMAABusy || card->DMABBusy)
		return -1;

	size >>= 3;		// 64-bit-words
	source >>= 2;		// 8 byte aligned data,
	destination >>= 2;	// now 19 bit 64-bit-word-address

	DecoderDelByte(card, 0x0C1, 0x06);	// DMA idle

	DecoderWriteByte(card, 0x0DA, (u8) ((source >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0D9, (u8) ((source >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0D8, (u8) (source & 0x000000FFL));
	DecoderWriteByte(card, 0x0D7,
			 (u8) ((destination >> 16) & 0x00000007L));
	DecoderWriteByte(card, 0x0D6,
			 (u8) ((destination >> 8) & 0x000000FFL));
	DecoderWriteByte(card, 0x0D5, (u8) (destination & 0x000000FFL));

	//card->DMAMoveBusy=1;  // would have to catch that in all the other DMA routines
	DecoderSetByte(card, 0x0C1, 0x06);	// DMA block move

	return 0;
}

  // size in words
 // align:  number of words on wich start of block will be aligned
// return value is 21 bit word address, or 0xFFFFFFFF on error
u32 DRAMAlloc(struct cvdv_cards * card, u32 size, int align)
{
	struct DRAMBlock *ptr, *ptr2;
	u32 addr = 0;
	u32 alignmask = align - 1;
	int valid = 0;
	if (!card || size == 0)
		return BLANK;
	if (size & 3)
		size = (size & ~3) + 4;	// increase size if not 64 bit aligned
	if (card->DRAMFirstBlock == NULL) {	// virgin territory?
		valid = ((addr + size) <= card->DRAMSize);	// does it fit at all?
	} else {
		addr = 0;
		valid = ((addr + size) <= card->DRAMSize);	// does it fit at all?
		for (ptr2 = card->DRAMFirstBlock;
		     (ptr2 != NULL) && (valid); ptr2 = ptr2->next) {	// check against all existing blocks
			if ((ptr2->start >= addr)
			    && (ptr2->start < (addr + size)))
				valid = 0;	// existing block start inside new block?
			else if (((ptr2->start + ptr2->length) > addr)
				 && ((ptr2->start + ptr2->length) <=
				     (addr + size)))
				valid = 0;	// existing block end inside new block?
			else if ((ptr2->start < addr)
				 && ((ptr2->start + ptr2->length) >
				     (addr + size))) valid = 0;	// new block inside existing block?
		}
		for (ptr = card->DRAMFirstBlock; (ptr != NULL) && (!valid);
		     ptr = ptr->next) {	// check all existing blocks
			addr = ptr->start + ptr->length;	// assume, after this block is free space
			if (addr & alignmask)
				addr = (addr & ~alignmask) + align;	// round up to alignation border
			valid = ((addr + size) <= card->DRAMSize);	// does it fit at all?
			for (ptr2 = card->DRAMFirstBlock;
			     (ptr2 != NULL) && (valid); ptr2 = ptr2->next) {	// check against all existing blocks
				if ((ptr2->start >= addr)
				    && (ptr2->start < (addr + size)))
					valid = 0;	// existing block start inside new block?
				else
				    if (
					((ptr2->start + ptr2->length) >
					 addr)
					&& ((ptr2->start + ptr2->length) <=
					    (addr + size)))
					valid = 0;	// existing block end inside new block?
				else if ((ptr2->start < addr)
					 && ((ptr2->start + ptr2->length) >
					     (addr + size)))
					valid = 0;	// new block inside existing block?
			}
		}
	}
	if (valid) {		// The new block fits
		ptr =
		    (struct DRAMBlock *) kmalloc(sizeof(struct DRAMBlock),
						 GFP_KERNEL);
		if (ptr == NULL) {
			printk(KERN_INFO LOGNAME
			       ": ERROR: out of kernel memory. Please reboot if possible.\n");
			return BLANK;	// out of kernel mem
		}
		if (card->DRAMFirstBlock == NULL) {
			card->DRAMFirstBlock = ptr;
		} else {
			ptr2 = card->DRAMFirstBlock;
			while (ptr2->next != NULL)
				ptr2 = ptr2->next;
			ptr2->next = ptr;
		}
		ptr->next = NULL;
		ptr->start = addr;
		ptr->length = size;
		MDEBUG(1,": DRAM Allocate 0x%08X-0x%08X\n", addr,
		       addr + size - 1);
		return addr;
	}
	return BLANK;
}

 // addr is the return value of that resp. DRAMAlloc call
// returns 0 on success (always)
int DRAMFree(struct cvdv_cards *card, u32 addr)
{
	struct DRAMBlock *ptr, *ptr2;
	ptr2 = NULL;
	for (ptr = card->DRAMFirstBlock; ptr != NULL; ptr = ptr->next) {	// check all existent blocks
		if (addr == ptr->start) {	// this is our block to be removed
			if (ptr2 == NULL)
				card->DRAMFirstBlock = ptr->next;
			else
				ptr2->next = ptr->next;
			if (ptr) kfree(ptr);
			MDEBUG(1, ": DRAM Free 0x%08X\n", addr);
		} else
			ptr2 = ptr;
	}
	return 0;
}

 // free all blocks
// returns 0 on success (always)
int DRAMRelease(struct cvdv_cards *card)
{
	struct DRAMBlock *ptr, *ptr2;
	MDEBUG(1, ": -- DRAMRelease\n");
	for (ptr = card->DRAMFirstBlock; ptr != NULL; ptr = ptr2) {	// check all existent blocks
		ptr2 = ptr->next;
		MDEBUG(4, ": kfree(0x%08X)\n",(int)ptr);
		kfree(ptr);
	}
	card->DRAMFirstBlock = NULL;
	return 0;
}
