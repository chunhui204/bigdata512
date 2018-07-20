/* 
    osd.c

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

     ////////////////////////////////////////////////////////////////
    //                                                            //
   //  Functions to Draw on the On Screen Display of the L64021  //
  //  CLUT-Mode with 2, 4, or 8 bit per pixel, up to 720*576    //
 //                                                            //
////////////////////////////////////////////////////////////////
// OSD Pixel Aspect Ratio:
// CCIR601 525 Lines (NTSC,PAL-M): 11/10 (100*100 appears as 100*110)
// CCIR601 625 Lines (PAL):        11/12 (100*100 appears as 100*91.6)
//
// OSD functions for external use:
//   int OSDOpen(struct cvdv_cards *card);
//   int OSDClose(struct cvdv_cards *card);
//   int OSDQuery(struct cvdv_cards *card, int *x0, int *y0, int *x1, int *y1, int *aspx, int *aspy);
//   int OSDStartPicture(struct cvdv_cards *card, int left, int top, int width, int height, int bit, int mix);
//   void OSDShow(struct cvdv_cards *card);
//   void OSDHide(struct cvdv_cards *card);
//   void OSDClear(struct cvdv_cards *card);
//   void OSDFill(struct cvdv_cards *card, int col);
//   int OSDSetColor(struct cvdv_cards *card, int num, int R, int G, int B, int mix, int trans);
//   int OSDSetPixel(struct cvdv_cards *card, int x, int y, int col);
//   int OSDGetPixel(struct cvdv_cards *card, int x, int y);
//   int OSDSetRow(struct cvdv_cards *card, int x0, int y, int x1, u8 *data);
//   int OSDFillRow(struct cvdv_cards *card, int x0, int y, int x1, int col);
//   void OSDLine(struct cvdv_cards *card, int x0, int y0, int x1, int y1, int col);
//
// Return codes: (unless otherwise specified)
//    0: OK
//   -1: Range error
//   -2: OSD not open
//

#define __NO_VERSION__

#include "osd.h"
#include "dram.h"
#include "l64021.h"

 // Builds a 4-word picture header in buf
// returns number of words in pixel field on success, -1 on error
int OSDHeader(u16 * buf,	// 4 words
	      int *bit,		// bit per pixel: 2, 4, or 8
	      int *startrow,	// position of our block, 
	      int *stoprow,	// row: 0..313
	      int *startcol,	// col: 0..864
	      int *stopcol,	//
	      int *mix,		// opacity for mixed pixel, 0..15 (0%..94% resp.)
	      int nopal)
{				// 1: use previous palette
	int count;
	if (buf != NULL) {
		if (*bit == 8)
			*bit = 1;
		else if (*bit == 2)
			*bit = 0;
		else
			*bit = 2;
		if (*startrow < 0)
			*startrow = 0;
		if (*startrow > 312)
			*startrow = 312;
		if (*stoprow <= *startrow)
			*stoprow = *startrow + 1;
		if (*stoprow > 313)
			*stoprow = 313;
		if (*startcol < 0)
			*startcol = 0;
		if (*startcol > 863)
			*startcol = 863;
		if (*stopcol <= *startcol)
			*stopcol = *startcol + 2;
		if (*stopcol > 864)
			*stopcol = 864;
		if ((*stopcol - *startcol + 1) & 1)
			(*stopcol)--;
		if (*mix < 0)
			*mix = 0;
		if (*mix > 15)
			*mix = 15;
		buf[0] = ((*bit << 14) & 0x8000) | (*startrow & 0x01FF);
		buf[1] =
		    ((*mix << 12) & 0xF000) | ((*bit << 11) & 0x0800) |
		    ((nopal) ? 0x0400 : 0x0000) | (*stoprow & 0x01FF);
		buf[2] = *startcol & 0x03FF;
		buf[3] = *stopcol & 0x03FF;
		count =
		    (*stoprow - *startrow + 1) * (*stopcol - *startcol +
						  1);
		if (*bit == 1) {
			count =
			    ((count >> 3) + ((count & 0x07) ? 1 : 0)) << 2;
			*bit = 8;
		} else if (*bit == 0) {
			count =
			    ((count >> 5) + ((count & 0x1F) ? 1 : 0)) << 2;
			*bit = 2;
		} else if (*bit == 2) {
			count =
			    ((count >> 4) + ((count & 0x0F) ? 1 : 0)) << 2;
			*bit = 4;
		}
		return count;	// word count of pixel data
	} else
		return -1;
}

// enables OSD mode
int OSDShow(struct cvdv_cards *card)
{
	if (card->OSD.open) {
		DecoderMaskByte(card, 0x109, 0x03, 0x01);
		DecoderDelByte(card, 0x112, 0x10);	// no filter
		return 0;
	} else
		return -2;
}

// disables OSD mode
int OSDHide(struct cvdv_cards *card)
{
	if (card->OSD.open) {
		DecoderMaskByte(card, 0x109, 0x03, 0x00);
		return 0;
	} else
		return -2;
}

// creates an empty picture in the memory of the card
// ONLY ONE PICTURE PER CARD!
// maximum sizes:     NTSC: 720*525  PAL: 720*576
// maximum positions: NTSC: 858*525  PAL: 864*625
// returns 0 on success, -1 on DRAM allocation error
int OSDStartPicture(struct cvdv_cards *card, int left, int top, int width,
		    int height, int bit, int mix)
{
	u16 TermHeader[] = { 0x01FF, 0x05FF, 0x0000, 0x0000 };
	u16 header[4];
	int size, pixelsize, palsize, frametop, startrow, stoprow,
	    startcol, stopcol;

	if (card->OSD.open)
		return -2;
	if (top & 1) {
		card->OSD.evenfirst = 0;
		card->OSD.evenheight = height / 2;
		card->OSD.oddheight = height - card->OSD.evenheight;
	} else {
		card->OSD.evenfirst = 1;
		card->OSD.oddheight = height / 2;
		card->OSD.evenheight = height - card->OSD.oddheight;
	}

	// Setting the picture for the lines in the even field
	frametop = top / 2;
	startrow = frametop;
	stoprow = frametop + card->OSD.evenheight - 1;
	startcol = left;
	stopcol = left + width - 1;
	pixelsize =
	    OSDHeader(header, &bit, &startrow, &stoprow, &startcol,
		      &stopcol, &mix, 0);
	card->OSD.evenheight = stoprow - startrow + 1;
	card->OSD.bpp = bit;
	if (bit == 8)
		palsize = 256;
	else if (bit == 2)
		palsize = 4;
	else
		palsize = 16;
	size = 8 + palsize + pixelsize;
	card->OSD.evenmem = DRAMAlloc(card, size, 32);
	if (card->OSD.evenmem == BLANK)
		return -1;
	card->OSD.evendata = card->OSD.evenmem;
	card->OSD.evenpalette = card->OSD.evendata + 4;
	card->OSD.evenbitmap = card->OSD.evenpalette + palsize;
	card->OSD.eventerm = card->OSD.evenbitmap + pixelsize;
	DecoderWriteWord(card, 0x110, (u16) (card->OSD.evendata >> 5));
	DRAMWriteWord(card, card->OSD.evendata, 4, header, 0);
	DRAMFillByte(card, card->OSD.evenpalette,
		     (palsize + pixelsize) * 2, 0x00);
	DRAMWriteWord(card, card->OSD.eventerm, 4, TermHeader, 0);

	// Setting the picture for the lines in the odd frame
	frametop += card->OSD.evenfirst;
	startrow = frametop;
	stoprow = frametop + card->OSD.oddheight - 1;
	pixelsize =
	    OSDHeader(header, &bit, &startrow, &stoprow, &startcol,
		      &stopcol, &mix, 0);
	card->OSD.oddheight = stoprow - startrow + 1;
	size = 8 + palsize + pixelsize;
	card->OSD.oddmem = DRAMAlloc(card, size, 32);
	if (card->OSD.oddmem == BLANK)
		return -1;
	card->OSD.odddata = card->OSD.oddmem;
	card->OSD.oddpalette = card->OSD.odddata + 4;
	card->OSD.oddbitmap = card->OSD.oddpalette + palsize;
	card->OSD.oddterm = card->OSD.oddbitmap + pixelsize;
	DecoderWriteWord(card, 0x10E, (u16) (card->OSD.odddata >> 5));
	DRAMWriteWord(card, card->OSD.odddata, 4, header, 0);
	DRAMFillByte(card, card->OSD.oddpalette, (palsize + pixelsize) * 2,
		     0x00);
	DRAMWriteWord(card, card->OSD.oddterm, 4, TermHeader, 0);

	// Update of the picture dimensions  
	card->OSD.width = stopcol - startcol + 1;
	card->OSD.height = card->OSD.evenheight + card->OSD.oddheight;
	card->OSD.open = 1;

	MDEBUG(1,": OSD Open %dX%d, %d bit, mem 0x%08X/0x%08X\n",
	       card->OSD.width, card->OSD.height, card->OSD.bpp,
	       card->OSD.evendata, card->OSD.odddata);
	return 0;
}

// Disables OSD and releases the buffers
// returns 0 on success, 1 on "not open"
int OSDClose(struct cvdv_cards *card)
{
	if (card->OSD.open) {
		OSDHide(card);
		DRAMFree(card, card->OSD.evenmem);
		DRAMFree(card, card->OSD.oddmem);
		card->OSD.open = 0;
		return 0;
	} else
		return -2;
}

// Opens OSD with this size and bit depth
// returns 0 on success, 1 on DRAM allocation error, 2 on "already open"
int OSDOpen(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
	    int bit, int mix)
{
	int ret;
	if (card->OSD.open)
		OSDClose(card);
	if (bit < 0)
		bit = 8;
	else if (bit < 2)
		bit = 2;
	else if (bit < 4)
		bit = 4;
	else
		bit = 8;
	if (x0 < 0)
		x0 = 0;
	if (x1 < 0)
		x1 = 720 - 1;
	if (x1 < x0)
		x1 = x0;
	if (y0 < 0)
		y0 = 0;
	if (y1 < 0)
		y1 = 576 - 1;
	if (y1 < y0)
		y1 = y0;
	if ((x1 + 1) > 720)
		x1 = 720 - 1;
	if (x0 > x1)
		x0 = x1;
	if (CCIR601Lines(card->videomode) == 625) {	// PAL
		if ((y1 + 1) > 576)
			y1 = 576 - 1;
		if (y0 > y1)
			y0 = y1;
		if (!
		    (ret =
		     OSDStartPicture(card, 134 + x0, 48 + y0, x1 - x0 + 1,
				     y1 - y0 + 1, bit, mix)))
			card->OSD.aspectratio = 12;	// pixel aspect ratio 12/11
	} else {		// NTSC
		if ((y1 + 1) > 484)
			y1 = 484 - 1;
		if (y0 > y1)
			y0 = y1;
		if (!
		    (ret =
		     OSDStartPicture(card, 126 + x0, 44 + y0, x1 - x0 + 1,
				     y1 - y0 + 1, bit, mix)))
			card->OSD.aspectratio = 10;	// pixel aspect ratio 10/11
	}
	return ret;
}

// fills parameters with the picture dimensions and the pixel aspect ratio (aspy=11)
int OSDQuery(struct cvdv_cards *card, int *x0, int *y0, int *x1, int *y1,
	     int *aspx)
{
	if (!card->OSD.open)
		return -2;
	*x0 = 0;
	*x1 = card->OSD.width - 1;
	*y0 = 0;
	*y1 = card->OSD.height - 1;
	*aspx = card->OSD.aspectratio;
	return 0;
}

// Sets all pixel to color 0
int OSDClear(struct cvdv_cards *card)
{
	if (!card->OSD.open)
		return -2;
	DRAMFillByte(card, card->OSD.oddbitmap,
		     (int) (card->OSD.oddterm - card->OSD.oddbitmap) * 2,
		     0x00);
	DRAMFillByte(card, card->OSD.evenbitmap,
		     (int) (card->OSD.eventerm - card->OSD.evenbitmap) * 2,
		     0x00);
	return 0;
}

// Sets all pixel to color <col>
int OSDFill(struct cvdv_cards *card, int col)
{
	u8 color;
	if (!card->OSD.open)
		return -2;
	if (card->OSD.bpp == 8) {
		color = col & 0xFF;
	} else if (card->OSD.bpp == 4) {
		color = (col & 0xF);
		color |= (color << 4);
	} else if (card->OSD.bpp == 2) {
		color = (col & 0x03);
		for (col = 1; col <= 3; col++)
			color |= (color << 2);
	} else
		color = 0x00;
	DRAMFillByte(card, card->OSD.oddbitmap,
		     (int) (card->OSD.oddterm - card->OSD.oddbitmap) * 2,
		     color);
	DRAMFillByte(card, card->OSD.evenbitmap,
		     (int) (card->OSD.eventerm - card->OSD.evenbitmap) * 2,
		     color);
	return 0;
}

// converts RGB(8 bit) to YCrCb(OSD format)
// mix: 0=opacity 100% 1=opacity at mix value
// trans: 0=mix bit applies 1=opacity 0%
// returns word in OSD palette format
u16 OSDColor(u8 R, u8 G, u8 B, int mix, int trans)
{
	u16 Y, Cr, Cb;
	Y = R * 77 + G * 150 + B * 29;	// Luma=0.299R+0.587G+0.114B 0..65535
	Cb = 2048 + B * 8 - (Y >> 5);	// Cr 0..4095
	Cr = 2048 + R * 10 - (Y >> 5);	// Cb 0..4095
	return ((trans) ? 0 :	// transparent pixel
		(Y & 0xFC00) |	// Luma 0..63
		((mix) ? 0x0100 : 0x0000) |	// Opacity applies
		((Cb >> 4) & 0x00F0) |	// Cb 0..15
		((Cr >> 8) & 0x000F)	// Cr 0..15
	    );
}

// set palette entry <num> to <r,g,b>, <mix> and <trans> apply
// R,G,B: 0..255
// RGB=1: R=Red, G=Green, B=Blue  RGB=0: R=Y G=Cb B=Cr
// mix=0, trans=0: pixel opacity 100% (only OSD pixel shows)
// mix=1, trans=0: pixel opacity as specified in header
// trans=1: pixel opacity 0% (only video pixel shows)
// returns 0 on success, 1 on error
int OSDSetColor(struct cvdv_cards *card, int num, int R, int G, int B,
		int YUV, int mix, int trans)
{
	u16 burst[4];		// minimal memory unit
	u32 addr;
	u16 color;
	if (!card->OSD.open)
		return -2;
	if (R < 0)
		R = 0;
	if (R > 255)
		R = 255;
	if (G < 0)
		G = 0;
	if (G > 255)
		G = 255;
	if (B < 0)
		B = 0;
	if (B > 255)
		B = 255;
	if ((num >= 0) && (num < (1 << card->OSD.bpp))) {
		if (num==0) MDEBUG(4,"OSD SetColor num=%d, R=%d, G=%d, B=%d, YUV=%d, mix=%d, trans=%d\n",
				   num,R,G,B,YUV,mix,trans);
		color = ((YUV)
			 ? ((trans) ? 0 : ((R << 8) & 0xFC00) |
			    ((mix) ? 0x0100 : 0x0000) | (G & 0x00F0) |
			    ((B >> 4) & 0x000F)) : OSDColor(R, G, B, mix,
							    trans));

		addr = card->OSD.oddpalette + num;
		DRAMReadWord(card, addr & ~3, 4, burst, 0);
		burst[addr & 3] = color;
		DRAMWriteWord(card, addr & ~3, 4, burst, 0);

		addr = card->OSD.evenpalette + num;
		DRAMReadWord(card, addr & ~3, 4, burst, 0);
		burst[addr & 3] = color;
		DRAMWriteWord(card, addr & ~3, 4, burst, 0);

		return 0;
	} else
		return -1;
}

// Set a number of entries in the palette
// sets the entries "firstcolor" through "lastcolor" from the array "data"
// data has 4 byte for each color:
// R,G,B, and a transparency value: 0->tranparent, 1..254->mix, 255->no mix
int OSDSetPalette(struct cvdv_cards *card, int firstcolor, int lastcolor,
		  u8 * data)
{
	u16 burst[4];		// minimal memory unit
	u32 addr;
	u16 color;
	int num, i = 0;
	if (!card->OSD.open)
		return -2;
	for (num = firstcolor; num <= lastcolor; num++)
		if ((num >= 0) && (num < (1 << card->OSD.bpp))) {
			color =
			    OSDColor(data[i], data[i + 1], data[i + 2],
				     ((data[i + 3] < 255) ? 1 : 0),
				     ((data[i + 3] == 0) ? 1 : 0));
			i += 4;

			addr = card->OSD.oddpalette + num;
			DRAMReadWord(card, addr & ~3, 4, burst, 0);
			burst[addr & 3] = color;
			DRAMWriteWord(card, addr & ~3, 4, burst, 0);

			addr = card->OSD.evenpalette + num;
			DRAMReadWord(card, addr & ~3, 4, burst, 0);
			burst[addr & 3] = color;
			DRAMWriteWord(card, addr & ~3, 4, burst, 0);
		}
	return 0;
}

// Sets transparency of mixed pixel (0..15)
int OSDSetTrans(struct cvdv_cards *card, int trans)
{
	u16 burst[4];		// minimal memory unit
	if (!card->OSD.open)
		return -2;
	trans &= 0x000F;
	DRAMReadWord(card, card->OSD.evendata, 4, burst, 0);
	burst[1] = (burst[1] & 0x0FFF) | (trans << 12);
	DRAMWriteWord(card, card->OSD.evendata, 4, burst, 0);

	DRAMReadWord(card, card->OSD.odddata, 4, burst, 0);
	burst[1] = (burst[1] & 0x0FFF) | (trans << 12);
	DRAMWriteWord(card, card->OSD.odddata, 4, burst, 0);
	return 0;
}

// sets pixel <x>,<y> to color number <col>
// returns 0 on success, 1 on error
int OSDSetPixel(struct cvdv_cards *card, int x, int y, int col)
{
	u16 burst[4];		// minimal memory unit od DRAM
	u32 addr;
	int offset, ppw, pos, shift, height, posmask;
	u16 mask;

	if (!card->OSD.open)
		return -2;
	if ((y & 1) == card->OSD.evenfirst) {	// even or odd frame?
		addr = card->OSD.oddbitmap;
		height = card->OSD.oddheight;
	} else {
		addr = card->OSD.evenbitmap;
		height = card->OSD.evenheight;
	}
	y >>= 1;
	if ((x >= 0) && (x < card->OSD.width) && (y >= 0) && (y < height)) {	// clipping
		ppw =
		    ((card->OSD.bpp == 4) ? 2 : ((card->OSD.bpp == 8) ? 1 : 3));	// OK, 4-(ln(bpp)/ln(2)) would have worked, too...
		pos = x + y * card->OSD.width;	// pixel number in bitfield
		addr += (pos >> ppw);	// 21 bit address of word with our pixel
		offset = addr & 3;	// offset in burst
		addr &= ~3;	// 21 bit burst address
		posmask = (1 << ppw) - 1;	// mask for position inside word
		shift = ((posmask - (pos & posmask)) << (4 - ppw));	// pixel shift inside word
		mask = (1 << (1 << (4 - ppw))) - 1;	// pixel mask
		DRAMReadWord(card, addr, 4, burst, 0);	// get the burst with our pixel...
		burst[offset] =
		    (burst[offset] & ~(mask << shift)) | ((col & mask) <<
							  shift);
		DRAMWriteWord(card, addr, 4, burst, 0);	// ...and write it back
		return 0;
	} else
		return -1;
}

// returns color number of pixel <x>,<y>,  or -1
int OSDGetPixel(struct cvdv_cards *card, int x, int y)
{
	u16 burst[4];		// minimal memory unit
	u32 addr;
	int offset, ppw, pos, shift, height, posmask;
	u16 mask;

	if (!card->OSD.open)
		return -2;
	if ((y & 1) == card->OSD.evenfirst) {	// even or odd frame?
		addr = card->OSD.oddbitmap;
		height = card->OSD.oddheight;
	} else {
		addr = card->OSD.evenbitmap;
		height = card->OSD.evenheight;
	}
	y >>= 1;
	if ((x >= 0) && (x < card->OSD.width) && (y >= 0) && (y < height)) {	// clipping
		ppw =
		    ((card->OSD.bpp == 4) ? 2 : ((card->OSD.bpp == 8) ? 1 : 3));	// OK, 4-(ln(bpp)/ln(2)) would have worked, too...
		pos = x + y * card->OSD.width;	// pixel number in bitfield
		addr += (pos >> ppw);	// 21 bit address of word with our pixel
		offset = addr & 3;	// offset in burst
		addr &= ~3;	// 21 bit burst address
		posmask = (1 << ppw) - 1;	// mask for position inside word
		shift = ((posmask - (pos & posmask)) << (4 - ppw));	// pixel shift inside word
		mask = (1 << (1 << (4 - ppw))) - 1;	// pixel mask
		DRAMReadWord(card, addr, 4, burst, 0);	// get the burst with our pixel...
		return (burst[offset] >> shift) & mask;	// ...and return it's value 
	} else
		return -1;
}

// fills pixels x0,y through  x1,y with the content of data[]
// returns 0 on success, -1 on clipping all pixel
int OSDSetRow(struct cvdv_cards *card, int x0, int y, int x1, u8 * data)
{
	u16 burst[4];		// minimal memory unit
	u32 addr, addr1, bitmap;
	int offset, offset1, ppw, pos, pos1, shift, shift0, shift1,
	    shiftstep, height, bpp, x, i, endburst, endword;
	u16 mask, posmask;

	if (!card->OSD.open)
		return -2;
	if ((y & 1) == card->OSD.evenfirst) {
		bitmap = card->OSD.oddbitmap;
		height = card->OSD.oddheight;
	} else {
		bitmap = card->OSD.evenbitmap;
		height = card->OSD.evenheight;
	}
	y >>= 1;
	if ((y >= 0) && (y < height)) {
		i = 0;
		if (x0 > x1) {
			x = x1;
			x1 = x0;
			x0 = x;
		}
		if ((x0 >= card->OSD.width) || (x1 < 0))
			return -1;
		if (x0 < 0) {
			i -= x0;
			x0 = 0;
		}
		if (x1 >= card->OSD.width)
			x1 = card->OSD.width - 1;
		bpp = card->OSD.bpp;	// bits per pixel
		ppw = ((bpp == 4) ? 2 : ((bpp == 8) ? 1 : 3));	// positional parameter
		mask = (1 << bpp) - 1;	// mask for one pixel
		posmask = (1 << ppw) - 1;	// mask for position inside word

		pos = x0 + (y * card->OSD.width);	// pixel number of first pixel
		pos1 = pos + x1 - x0;	// pixel number of last pixel
		shift0 = ((posmask - (pos & posmask)) << (4 - ppw));
		shift1 = ((posmask - (pos1 & posmask)) << (4 - ppw));
		shiftstep = 1 << (4 - ppw);

		addr = bitmap + (pos >> ppw);	// DRAM address of word with first pixel
		addr1 = bitmap + (pos1 >> ppw);	//  "      "    "    "   "   last    "
		offset = (int) (addr & 3);	// word position inside burst
		offset1 = (int) (addr1 & 3);	// number of last word in the last burst
		addr &= ~3;	// burst address
		addr1 &= ~3;	// burst address of last pixel

		endburst = (addr1 != addr);	// end in other burst
		endword = (offset1 != offset);	// end in other word

		// read old content of first burst if the row start after the beginning or 
		// end before the end of the first burst
		if (offset || (pos & posmask) ||
		    (!endburst
		     && ((offset1 != 3)
			 || ((pos1 & posmask) != posmask)))) {
			DRAMReadWord(card, addr, 4, burst, 0);
		}
		// End beyond or at the end of this word?
		if (endburst || endword || ((pos1 & posmask) == posmask)) {
			// Fill first word
			for (shift = shift0; shift >= 0; shift -= shiftstep) {	// bit position inside word
				burst[offset] =
				    (burst[offset] & ~(mask << shift)) |
				    ((data[i++] & mask) << shift);
			}
			if (endburst || endword) {	// Any more words to fill?
				shift0 = posmask << (4 - ppw);	// from here on, we start at the beginning of each word
				offset++;	// fill the rest of the burst
				if (endburst) {	// end not in this burst?
					while (offset <= 3) {	// fill remaining words
						burst[offset] = 0x0000;	// clear first
						for (shift = shift0;
						     shift >= 0;
						     shift -= shiftstep) {
							burst[offset] |=
							    ((data
							      [i++] & mask)
							     << shift);
						}
						offset++;
					}
					DRAMWriteWord(card, addr, 4, burst, 0);	// write first burst
					addr += 4;	// go on to the next burst
					while (addr < addr1) {	// all bursts between start and end burst
						for (offset = 0;
						     offset <= 3; offset++) {	// 4 words per burst
							burst[offset] = 0x0000;	// clear first
							for (shift =
							     shift0;
							     shift >= 0;
							     shift -=
							     shiftstep) {
								burst
								    [offset]
								    |=
								    ((data
								      [i++]
								      &
								      mask)
								     <<
								     shift);
							}
						}
						DRAMWriteWord(card, addr,
							      4, burst, 0);	// write full burst
						addr += 4;	// next burst
					}
					offset = 0;
					if ((offset1 < 3) || shift1) {	// does the row ends before the end of the burst?
						DRAMReadWord(card, addr, 4,
							     burst, 0);	// then we have to read the old content
					}
				}
				while (offset < offset1) {	// end not in this word
					burst[offset] = 0x0000;	// clear first
					for (shift = shift0; shift >= 0;
					     shift -= shiftstep) {
						burst[offset] |=
						    ((data[i++] & mask) <<
						     shift);
					}
					offset++;
				}
				for (shift = shift0; shift >= shift1;
				     shift -= shiftstep) {	// last word
					burst[offset] =
					    (burst[offset] &
					     ~(mask << shift)) |
					    ((data[i++] & mask) << shift);
				}
			}
		} else {	// row starts and ends in one word
			for (shift = shift0; shift >= shift1; shift -= shiftstep) {	// bit position inside word
				burst[offset] =
				    (burst[offset] & ~(mask << shift)) |
				    ((data[i++] & mask) << shift);
			}
		}
		DRAMWriteWord(card, addr, 4, burst, 0);	// write only/last burst
		return 0;
	} else
		return -1;
}

// fills pixels x0,y0 through  x1,y1 with the content of data[]
// inc contains the width of one line in the data block,
// inc<=0 uses blockwidth as linewidth
// returns 0 on success, -1 on clipping all pixel
int OSDSetBlock(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
		int inc, u8 * data)
{
	int i, w = x1 - x0 + 1, ret = 0;
	if (inc > 0)
		w = inc;
	for (i = y0; i <= y1; i++) {
		ret |= OSDSetRow(card, x0, i, x1, data);
		data += w;
	}
	return ret;
}

// fills pixels x0,y through  x1,y with the color <col>
// returns 0 on success, -1 on clipping all pixel
int OSDFillRow(struct cvdv_cards *card, int x0, int y, int x1, int col)
{
	u16 burst[4];		// minimal memory unit
	u32 addr, addr1, bitmap;
	int offset, offset1, ppw, pos, pos1, shift, shift0, shift1,
	    shiftstep, height, bpp, x, i, endburst, endword;
	u16 mask, posmask;

	if (!card->OSD.open)
		return -2;
	if ((y & 1) == card->OSD.evenfirst) {
		bitmap = card->OSD.oddbitmap;
		height = card->OSD.oddheight;
	} else {
		bitmap = card->OSD.evenbitmap;
		height = card->OSD.evenheight;
	}
	y >>= 1;
	if ((y >= 0) && (y < height)) {
		i = 0;
		if (x0 > x1) {
			x = x1;
			x1 = x0;
			x0 = x;
		}
		if ((x0 >= card->OSD.width) || (x1 < 0))
			return -1;
		if (x0 < 0) {
			i -= x0;
			x0 = 0;
		}
		if (x1 >= card->OSD.width)
			x1 = card->OSD.width - 1;
		bpp = card->OSD.bpp;	// bits per pixel
		ppw = ((bpp == 4) ? 2 : ((bpp == 8) ? 1 : 3));	// positional parameter
		mask = (1 << bpp) - 1;	// mask for one pixel
		posmask = (1 << ppw) - 1;	// mask for position inside word

		pos = x0 + (y * card->OSD.width);	// pixel number of first pixel
		pos1 = pos + x1 - x0;	// pixel number of last pixel
		shift0 = ((posmask - (pos & posmask)) << (4 - ppw));
		shift1 = ((posmask - (pos1 & posmask)) << (4 - ppw));
		shiftstep = 1 << (4 - ppw);

		addr = bitmap + (pos >> ppw);	// DRAM address of word with first pixel
		addr1 = bitmap + (pos1 >> ppw);	//  "      "    "    "   "   last    "
		offset = (int) (addr & 3);	// word position inside burst
		offset1 = (int) (addr1 & 3);	// number of last word in the last burst
		addr &= ~3;	// burst address
		addr1 &= ~3;	// burst address of last pixel

		endburst = (addr1 != addr);	// end in other burst
		endword = (offset1 != offset);	// end in other word

		// read old content of first burst if the row start after the beginning or 
		// end before the end of the first burst
		if (offset || (pos & posmask) ||
		    (!endburst
		     && ((offset1 != 3)
			 || ((pos1 & posmask) != posmask)))) {
			DRAMReadWord(card, addr, 4, burst, 0);
		}
		if (endburst || endword || ((pos1 & posmask) == posmask)) {	// end beyond or at the end of this word?
			for (shift = shift0; shift >= 0; shift -= shiftstep) {	// bit position inside word
				burst[offset] =
				    (burst[offset] & ~(mask << shift)) |
				    ((col & mask) << shift);
			}
			if (endburst || endword) {
				shift0 = posmask << (4 - ppw);	// from here on, we start at the beginning of each word
				offset++;	// fill the rest of the burst
				if (endburst) {	// end not in this burst?
					while (offset <= 3) {	// fill remaining words
						burst[offset] = 0x0000;	// clear first
						for (shift = shift0;
						     shift >= 0;
						     shift -= shiftstep) {
							burst[offset] |=
							    ((col & mask)
							     << shift);
						}
						offset++;
					}
					DRAMWriteWord(card, addr, 4, burst, 0);	// write first burst
					addr += 4;	// next burst
					while (addr < addr1) {	// write all the bursts between start and end burst
						for (offset = 0;
						     offset <= 3; offset++) {
							burst[offset] =
							    0x0000;
							for (shift =
							     shift0;
							     shift >= 0;
							     shift -=
							     shiftstep) {
								burst
								    [offset]
								    |=
								    ((col
								      &
								      mask)
								     <<
								     shift);
							}
						}
						DRAMWriteWord(card, addr,
							      4, burst, 0);
						addr += 4;
					}
					offset = 0;
					if ((offset1 < 3) || shift1) {	// does the row ends before the end of the burst?
						DRAMReadWord(card, addr, 4,
							     burst, 0);	// then we have to read the old content
					}
				}
				while (offset < offset1) {	// end not in this word
					burst[offset] = 0x0000;
					for (shift = shift0; shift >= 0;
					     shift -= shiftstep) {
						burst[offset] |=
						    ((col & mask) <<
						     shift);
					}
					offset++;
				}
				for (shift = shift0; shift >= shift1;
				     shift -= shiftstep) {
					burst[offset] =
					    (burst[offset] &
					     ~(mask << shift)) | ((col &
								   mask) <<
								  shift);
				}
			}
		} else {	// row starts and ends in one word
			for (shift = shift0; shift >= shift1; shift -= shiftstep) {	// bit position inside word
				burst[offset] =
				    (burst[offset] & ~(mask << shift)) |
				    ((col & mask) << shift);
			}
		}
		DRAMWriteWord(card, addr, 4, burst, 0);
		return 0;
	} else
		return -1;
}

// fills pixels x0,y0 through  x1,y1 with the color <col>
// returns 0 on success, -1 on clipping all pixel
int OSDFillBlock(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
		 int col)
{
	int i, ret = 0;
	for (i = y0; i <= y1; i++)
		ret |= OSDFillRow(card, x0, i, x1, col);
	return ret;
}

// draw a line from x0,y0 to x1,y1 with the color <col>
int OSDLine(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
	    int col)
{
	int ct, ix, iy, ax, ay, dx, dy, off;
#define sgn(a) ((a)?(((a)>0)?1:-1):0)
	if (!card->OSD.open)
		return -2;
	dx = x1 - x0;
	dy = y1 - y0;
	if (dx == 0) {
		if (dy < 0)
			for (iy = y1; iy <= y0; iy++)
				OSDSetPixel(card, x0, iy, col);
		else
			for (iy = y0; iy <= y1; iy++)
				OSDSetPixel(card, x0, iy, col);
	} else if (dy == 0) {
		OSDFillRow(card, x0, y0, x1, col);
	} else {
		ay = 0;
		ax = 0;
		ix = sgn(dx);
		dx = abs(dx);
		iy = sgn(dy);
		dy = abs(dy);
		if (dx < dy) {
			off = dx;
			dx = dy;
			dy = off;
			ay = ix;
			ax = iy;
			ix = 0;
			iy = 0;
		}
		off = dx >> 1;
		ct = 1;
		OSDSetPixel(card, x0, y0, col);
		x1 = x0;
		y1 = y0;
		while (dx >= ct) {
			x0 += ix;
			y0 += ax;
			ct++;
			off += dy;
			if (off > dx) {
				off -= dx;
				x0 += ay;
				y0 += iy;
			}
			if (ax) {
				OSDSetPixel(card, x0, y0, col);
			} else {
				if (y0 != y1) {
					OSDFillRow(card, x1, y1, x0 - ay,
						   col);
					x1 = x0;
					y1 = y0;
				}
			}
		}
		if (!ax)
			OSDFillRow(card, x1, y0, x0, col);
	}
	return 0;
}
