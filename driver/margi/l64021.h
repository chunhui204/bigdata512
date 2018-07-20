/* 
    l64021.h

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

#ifndef _L64021_H_
#define _L64021_H_

#include "margi.h"
#include "l64014.h"
// L64021 DRAM definitions

#define DRAMMaxSize 0x00200000	// 2 MWords of DRAM

// definitions for the L64021

#define DECODER_OFFSET          0x400

#define L21INTR0  0x000
#define L21INTR1  0x001
#define L21INTR2  0x002
#define L21INTR3  0x003
#define L21INTR4  0x004


// Host interface registers

// Video Decoder Registers

// CSS Regs

// Memory Interface

// Microcontroller

// Video Interface

// Audio Decoder

// RAM Test

// SPU Decoder




    ////////////////////////////////////////////////////
   //                                                //
  //  Access to the L64021 registers (0x400-0x7FF)  //
 //                                                //
////////////////////////////////////////////////////

#define DecoderWriteByte(card,where,what) WriteByte(card,where,what)
#define DecoderReadByte(card,where) ReadByte(card,where)
#define DecoderMaskByte(card,where,mask,bits) MaskByte(card,where,mask,bits)
#define DecoderSetByte(card,addr,bits) DecoderWriteByte(card,addr,DecoderReadByte(card,addr)|(bits))
#define DecoderDelByte(card,addr,mask) DecoderWriteByte(card,addr,DecoderReadByte(card,addr)&~(mask))

#define DecoderReadWord(card,addr) ((u16)DecoderReadByte(card,addr)|\
				    ((u16)DecoderReadByte(card,(addr)+1)<<8))

#define DecoderWriteWord(card,addr,data) {\
        DecoderWriteByte(card,addr,(data) & 0xFF);\
        DecoderWriteByte(card,(addr)+1,((data)>>8) & 0xFF);}


#define DecoderReadMWord(card, addr)(\
          (u32)DecoderReadByte(card,addr)|\
	  ((u32)DecoderReadByte(card,(addr)+1)<<8)|\
	  ((u32)DecoderReadByte(card,(addr)+2)<<16))

#define DecoderWriteMWord(card,addr,data) {\
  DecoderWriteByte(card,addr,(data) & 0xFF);\
  DecoderWriteByte(card,(addr)+1,((data)>>8) & 0xFF);\
  DecoderWriteByte(card,(addr)+2,((data)>>16) & 0xFF);}

#define DecoderReadDWord(card,addr) (\
           (u32)DecoderReadByte(card,addr)|\
	   ((u32)DecoderReadByte(card,(addr)+1)<<8)|\
	   ((u32)DecoderReadByte(card,(addr)+2)<<16)|\
	   ((u32)DecoderReadByte(card,(addr)+3)<<24))

#define DecoderWriteDWord(card,addr,data) {\
  DecoderWriteByte(card,addr,(data) & 0xFF);\
  DecoderWriteByte(card,(addr)+1,((data)>>8) & 0xFF);\
  DecoderWriteByte(card,(addr)+2,((data)>>16) & 0xFF);\
  DecoderWriteByte(card,(addr)+3,((data)>>24) & 0xFF);}


void l64020Reset(struct cvdv_cards *card);


#endif				// _L64021_H_
