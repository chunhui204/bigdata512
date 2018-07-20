/* 
    osd.h

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

#ifndef CVDV_OSD_H
#define CVDV_OSD_H

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

#include "cardbase.h"

// enables OSD mode
int OSDShow(struct cvdv_cards *card);

// disables OSD mode
int OSDHide(struct cvdv_cards *card);

// creates an empty picture in the memory of the card
// ONLY ONE PICTURE PER CARD!  ( might be changed in the future, if i find time...)
// maximum sizes:     NTSC: 720*525  PAL: 720*576
// maximum positions: NTSC: 858*525  PAL: 864*625
// returns 0 on success, -1 on DRAM allocation error
int OSDStartPicture(struct cvdv_cards *card, int left, int top, int width,
		    int height, int bit, int mix);

// Disables OSD and releases the buffers
// returns 0 on success, 1 on "not open"
int OSDClose(struct cvdv_cards *card);

// Opens OSD with this size and bit depth
// returns 0 on success, 1 on DRAM allocation error, 2 on "already open"
int OSDOpen(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
	    int bit, int mix);

// fills parameters with the picture dimensions and the pixel aspect ratio (aspy=11)
int OSDQuery(struct cvdv_cards *card, int *x0, int *y0, int *x1, int *y1,
	     int *aspx);

// Sets all pixel to color 0
int OSDClear(struct cvdv_cards *card);

// Sets all pixel to color <col>
int OSDFill(struct cvdv_cards *card, int col);

// converts RGB(8 bit) to YCrCb(OSD format)
// mix: 0=opacity 100% 1=opacity at mix value
// trans: 0=mix bit applies 1=opacity 0%
// returns word in OSD palette format
u16 OSDColor(u8 R, u8 G, u8 B, int mix, int trans);

// set palette entry <num> to <r,g,b>, <mix> and <trans> apply
// R,G,B: 0..255
// RGB=1: R=Red, G=Green, B=Blue  RGB=0: R=Y G=Cb B=Cr
// mix=0, trans=0: pixel opacity 100% (only OSD pixel shows)
// mix=1, trans=0: pixel opacity as specified in header
// trans=1: pixel opacity 0% (only video pixel shows)
// returns 0 on success, 1 on error
int OSDSetColor(struct cvdv_cards *card, int num, int R, int G, int B,
		int YUV, int mix, int trans);

// Set a number of entries in the palette
// sets the entries "firstcolor" through "lastcolor" from the array "data"
// data has 4 byte for each color:
// R,G,B, and a transparency value: 0->tranparent, 1..254->mix, 255->no mix
int OSDSetPalette(struct cvdv_cards *card, int firstcolor, int lastcolor,
		  u8 * data);

// Sets transparency of mixed pixel (0..15)
int OSDSetTrans(struct cvdv_cards *card, int trans);

// sets pixel <x>,<y> to color number <col>
// returns 0 on success, 1 on error
int OSDSetPixel(struct cvdv_cards *card, int x, int y, int col);

// returns color number of pixel <x>,<y>,  or -1
int OSDGetPixel(struct cvdv_cards *card, int x, int y);

// fills pixels x0,y through  x1,y with the content of data[]
// returns 0 on success, -1 on clipping all pixel
int OSDSetRow(struct cvdv_cards *card, int x0, int y, int x1, u8 * data);

// fills pixels x0,y0 through  x1,y1 with the content of data[]
// inc contains the width of one line in the data block,
// inc<=0 uses blockwidth as linewidth
// returns 0 on success, -1 on clipping all pixel
int OSDSetBlock(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
		int inc, u8 * data);

// fills pixels x0,y through  x1,y with the color <col>
// returns 0 on success, -1 on clipping all pixel
int OSDFillRow(struct cvdv_cards *card, int x0, int y, int x1, int col);

// fills pixels x0,y0 through  x1,y1 with the color <col>
// returns 0 on success, -1 on clipping all pixel
int OSDFillBlock(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
		 int col);

// draw a line from x0,y0 to x1,y1 with the color <col>
int OSDLine(struct cvdv_cards *card, int x0, int y0, int x1, int y1,
	    int col);

#endif				/* CVDV_OSD_H */
