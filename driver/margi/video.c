/* 
    video.c

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

//
//  Video Decoder
//
#define __NO_VERSION__

#include "video.h"
#include "l64021.h"
#include "dram.h"

// Set the background of the OSD and SPU and it's color
// mode=0: Video on Black
// mode=1: Black
// mode=2: Selected Color
// mode=3: Video on Selected Color
void VideoSetBackground(struct cvdv_cards *card, int mode, u8 Y, u8 Cb,
			u8 Cr)
{
	DecoderWriteByte(card, 0x10A, Y);
	DecoderWriteByte(card, 0x10B, Cb);
	DecoderWriteByte(card, 0x10C, Cr);
	DecoderMaskByte(card, 0x109, 0xC0, mode << 6);
}


int DecoderStartDecode(struct cvdv_cards *card)
{
	DecoderSetByte(card, 0x0F6, 0x01);
	if (card->audiostate.AV_sync_state)
		card->videosync = 1;
	return 0;
}

int DecoderStopDecode(struct cvdv_cards *card)
{
	DecoderDelByte(card, 0x0F6, 0x01);
	card->videosync = 0;
	return 0;
}

// Sets Display Override (Still Image Display) to Frame Buffer at specified addresses,
// addresses are 16 bit, in 64 byte resolution
// mode: 0=off, 1=Frame, 2=Field
// width: width of the still picture in 8 pixel units
int DecoderStillImageDisplay(struct cvdv_cards *card, int mode, int width,
			     u16 LumaAddr, u16 ChromaAddr)
{
	DecoderStopDecode(card);
	DecoderWriteWord(card, 0x11D, LumaAddr);
	DecoderWriteWord(card, 0x11F, ChromaAddr);
	DecoderWriteByte(card, 0x11B, width & 0x7F);
	DecoderMaskByte(card, 0x109, 0x30, (mode & 3) << 4);	// Display Override Mode
	return 0;
}

// Frees allocated frame buffers
int DecoderKillFrameBuffers(struct cvdv_cards *card)
{
	MDEBUG(1, ": -- DecoderKillFrameBuffers\n");
	DecoderStopDecode(card);
	DRAMFree(card, card->FrameStoreLuma1);
	card->FrameStoreLuma1 = BLANK;
	DRAMFree(card, card->FrameStoreChroma1);
	card->FrameStoreChroma1 = BLANK;
	DRAMFree(card, card->FrameStoreLuma2);
	card->FrameStoreLuma2 = BLANK;
	DRAMFree(card, card->FrameStoreChroma2);
	card->FrameStoreChroma2 = BLANK;
	DRAMFree(card, card->FrameStoreLumaB);
	card->FrameStoreLumaB = BLANK;
	DRAMFree(card, card->FrameStoreChromaB);
	card->FrameStoreChromaB = BLANK;
	card->FrameBuffersAllocated = 0;
//  DecoderWriteWord(
	return 0;
}

int DecoderSetFrameBuffers(struct cvdv_cards *card, int lines,	// number of lines of the decoded MPEG
			   int TwoFrames,	// 1 if no B-Frames are present in the video stream, thus allowing only 2 framestores
			   int RMM)	// 1 if RMM
{
#define SEGMENTS 44		// 40..54 for PAL, 44 recommended
#define BUFFERSET(buf,adr,align)  if (buf>0) {\
	if (buf&((1<<align)-1)) buf=(buf&~((1<<align)-1))+(1<<align);\
	addr=DRAMAlloc(card,buf,1<<align);\
	if (addr==BLANK) return adr;\
	card->buf=addr;\
	addr>>=align;\
	DecoderWriteByte(card,adr,addr&0xFF);\
	DecoderWriteByte(card,adr+1,(addr>>8)&(0x00FF));\
}
	u32 addr;
	int pixel, byteperline;	// visible pixel per video line, same for PAL and NTSC
	int FrameStoreLuma1, FrameStoreChroma1,
	    FrameStoreLuma2, FrameStoreChroma2,
	    FrameStoreLumaB, FrameStoreChromaB;
	MDEBUG(1, ": -- DecoderSetFrameBuffers\n");
	DecoderStopDecode(card);
	//DecoderStopChannel(card);
	//lines=((CCIR601Lines(card->videomode)==625)?576:480);
	byteperline = (DecoderReadByte(card, 0x116) & 0x7F) * 8;	// main 64-bit reads per line
	pixel = byteperline * lines;
	FrameStoreLuma1 = FrameStoreLuma2 = FrameStoreLumaB = pixel >> 1;	// 8 bit luma per pixel  in words
	FrameStoreChroma1 = FrameStoreChroma2 = FrameStoreChromaB =
	    pixel >> 2;		// 8+8 bit chroma every 2nd pixel every 2nd line
	if (card->FrameBuffersAllocated)
		DecoderKillFrameBuffers(card);
	BUFFERSET(FrameStoreLuma1, 0x0E0, 5);	// Anchor Frame Store 1
	BUFFERSET(FrameStoreChroma1, 0x0E2, 5);
	BUFFERSET(FrameStoreLuma2, 0x0E4, 5);	// Anchor Frame Store 2
	BUFFERSET(FrameStoreChroma2, 0x0E6, 5);
	if (TwoFrames) {
		DecoderDelByte(card, 0x0F8, 0x01);
	} else {
//    if (CCIR601Lines(card->videomode)==525) {  // Normal Mode, NTSC
		if (!RMM) {	// Normal Mode, NTSC
			BUFFERSET(FrameStoreLumaB, 0x0E8, 5);	// B Frame Store
			BUFFERSET(FrameStoreChromaB, 0x0EA, 5);
			DecoderDelByte(card, 0x0F8, 0x01);
		} else {	// Reduced Memory Mode, PAL
			// 44 segments with 8 lines each (8 bit luma + 4 bit chroma)
			// only display modes 4-8, 10, and 11 are allowed
			FrameStoreLumaB =
			    (8 * byteperline * SEGMENTS) >> 1;
			FrameStoreChromaB =
			    (4 * byteperline * SEGMENTS) >> 1;
			BUFFERSET(FrameStoreLumaB, 0x0E8, 5);	// B Frame Store
			BUFFERSET(FrameStoreChromaB, 0x0EA, 5);
			DecoderWriteByte(card, 0x121, SEGMENTS << 1);	// Number of segments
			DecoderSetByte(card, 0x0F8, 0x01);
		}
	}
	card->FrameBuffersAllocated = 1;
#undef SEGMENTS
#undef BUFFERSET
	return 0;
}

// returns size of the Video ES Buffer in bytes or 0=error
u32 DecoderGetVideoESSize(struct cvdv_cards * card)
{
	if (!card->ChannelBuffersAllocated)
		return 0;	// buffer not initialised
	return (u32) ((DecoderReadWord(card, 0x04A) & 0x3FFF) -
		      (DecoderReadWord(card, 0x048) & 0x3FFF)) * 256;	// bytes
}

// returns level of fullness in bytes
u32 DecoderGetVideoESLevel(struct cvdv_cards * card)
{
	u32 items;
	items = DecoderReadByte(card, 0x086);
	items |= ((DecoderReadWord(card, 0x087) & 0x07FF) << 8);
	items *= 8;		// 64 bit per item
	return items;
}

// pics=0 --> items=bytes
// pics=1 --> items=pictures
void DecoderSetVideoPanic(struct cvdv_cards *card, int pics, int items)
{
	if (pics < 0) {
		DecoderMaskByte(card, 0x045, 0x18, 0x00 << 3);	// disable panic mode
	} else {
		if (pics) {
			DecoderWriteMWord(card, 0x086, items & 0x0003FFFF);
			DecoderMaskByte(card, 0x045, 0x18, 0x02 << 3);	// set panic mode to "number of pictures" in VideoES
		} else {
			DecoderWriteMWord(card, 0x086,
					  (items / 8) & 0x0003FFFF);
			DecoderMaskByte(card, 0x045, 0x18, 0x01 << 3);	// set panic mode to "number of 8-byte-frames" in VideoES
		}
	}
}

int DecoderClose(struct cvdv_cards *card)
{
	if (card->DecoderOpen) {
		MDEBUG(1, ": -- DecoderClose\n");
		DecoderStopDecode(card);
		DecoderKillFrameBuffers(card);
		card->DecoderOpen = 0;
		card->lastvattr = 0;
		return 0;
	} else
		return 1;
}

// returns 0 on success, 1 on "picture size too big", 2 on "out of DRAM memory"
int DecoderOpen(struct cvdv_cards *card, int x, int y,	// size of the decoded MPEG picture
		int aspect,	// pixel or picture aspect ratio of the MPEG picture: 1=square pixel 2=3:4 3=9:16 4=1:2.21
		int Field,	// 0:Frame (interlaced, MPEG-2) , 1:Field (non-interlaced, MPEG-1) structure
		int Letterbox,	// 0:PanScan (4:3), 1:letterbox (16:9, 8:3) picture ratio
		int RMM)	// 1:use ReducedMemoryMode
{
	int mode,		// Display Mode
	 i, factor,		// zoom factor
	 top, bottom, left, right, width, height, newwidth, newheight,	// screen size
	 vaspx, vaspy,		// output video pixel aspect ratio
	 paspx, paspy,		// input picture pixel aspect ratio
	 SIF;			// 0:Full (480/576 lines, MPEG-2), 1:SIF (half, 240/288 lines, MPEG-1) resolution

	MDEBUG(1, ": -- DecoderOpen x:%d y:%d asp:%d field:%d lt:%d rmm:%d\n",
	       x, y, aspect, Field, Letterbox, RMM);
	if ((x <= 0) || (y <= 0))
		return 4;	// picture too small
//if (card->DecoderOpen) return 3;
	DecoderStopDecode(card);
	DecoderClose(card);	// closes only, if already open
	vaspy = 11;
	vaspx = ((CCIR601Lines(card->videomode) == 525) ? 10 : 12);	// screen pixel aspect ratio
	// note: this aspect ratio applies to 704 pixel width, but the card's default is 720, wich is not 3:4 picture aspect ratio anymore!?
	i = ((x == 720) ? 704 : x);	// 720 wide is overscan of 704 wide
	switch (aspect) {	// MPEG data pixel aspect ratio
	case 1:
		paspx = 1;
		paspy = 1;
		break;
	default:
	case 2:
		paspx = 4 * y;
		paspy = 3 * i;
		break;
	case 3:
		paspx = 16 * y;
		paspy = 9 * i;
		break;
	case 4:
		paspx = 221 * y;
		paspy = 100 * i;
		break;
	}
	top =
	    DecoderReadByte(card,
			    0x129) | ((DecoderReadByte(card, 0x12B) & 0x07)
				      << 8);	// current Start- and End Column
	bottom =
	    DecoderReadByte(card,
			    0x12A) | ((DecoderReadByte(card, 0x12B) & 0x70)
				      << 4);
	height = (bottom - top + 1) * 2;	// screen (frame) height
	left =
	    DecoderReadByte(card,
			    0x12C) | ((DecoderReadByte(card, 0x12E) & 0x07)
				      << 8);	// current Start- and End Row
	right =
	    DecoderReadByte(card,
			    0x12D) | ((DecoderReadByte(card, 0x12E) & 0x70)
				      << 4);
	width = (right - left + 1) / 2;	// screen width, 2 clocks = 1 pixel

	if (RMM)
		DecoderSetByte(card, 0x0F8, 0x01);
	else
		DecoderDelByte(card, 0x0F8, 0x01);

	DecoderWriteByte(card, 0x0EF, 0x08);

	//if (x>width) {  // Is the picture too wide for the screen?
	//  DecoderSetByte(card,0x112,0x40);  // Horiz. 2:1 Filter enable
	//  x/=2;
	//} else {
	DecoderDelByte(card, 0x112, 0x40);	// Horiz. 2:1 Filter disable
	//}




	if (1 /*Letterbox */ ) {	// Fit to width, reduce height
		newwidth = (x * vaspy * paspx / (paspy * vaspx));	// width in right aspect ratio
		if (newwidth <= 360) {	// less then about half the screen size?
			SIF = 1;
			newwidth *= 2;
		} else {
			SIF = 0;
		}
		if ((newwidth == 704) || (newwidth == 720))
			width = newwidth;	// standard sizes?
		newheight =
		    (y * vaspx * paspy / (paspx * vaspy)) * width / x;
		factor = newheight * 100 / y;
		printk(KERN_INFO LOGNAME
		       ": Decoder Open: Display size %d x %d, Picture size %d x %d, Demanded size: %d x %d, factor %d\n",
		       width, height, x, y, newwidth, newheight, factor);
		// 16:9 Letterbox
		if ((aspect == 3)
		    || ((aspect == 0)
			&& (((factor >= 65) && (factor <= 80))
			    || ((factor >= 140) && (factor <= 160))))) {
			if (SIF) {	// height * 1.5, SIF Letterbox
				if (RMM)
					return 1;	// not supported!
				height = (y * 3) / 2 - 2;
				mode = 3;
			} else {	// height * 0.75, 16:9 Letterbox
				height = (y * 3) / 4 - 2;
				mode = 8;
			}
			// 2.21:1 Letterbox
		} else if ((aspect == 4)
			   || ((aspect == 0)
			       && (((factor >= 45) && (factor <= 60))
				   || (SIF && ((factor >= 90)
					       && (factor <= 110)))))) {
			if (SIF) {	// height * 1
				height = y;
				mode = 5;
			} else {	// height / 2
				height = y / 2;
				mode = 11;
			}
			// 3:4 aspect ratio
		} else {
			if (SIF) {
				height = y * 2;
				mode = ((Field && ~RMM) ? 9 : 10);
			} else if (newwidth > 720) {	// picture too wide, scale down to 3/4
				height = (y * 3) / 4;
				mode = 8;
			} else {
				height = y;
				mode = ((Field) ? 7 : 5);
//        mode=((Field)?5:7);
			}
		}
		width = (x * vaspy * paspx / (paspy * vaspx)) * height / y;
		if (x < width) {	// does the picture needs a horizontal blow-up?
			DecoderWriteByte(card, 0x115,
					 ((x * 256 + width - 1) / width) & 0xFF);	// Horiz.Filter scale, x/width*256, rounded up
			DecoderSetByte(card, 0x114, 0x02);	// Horiz.Filter enable
		} else if (x == width) {
			DecoderWriteByte(card, 0x115, 0);	// 1:1 scale
			DecoderDelByte(card, 0x114, 0x02);	// Horiz.Filter disable
		} else if (x <= 720) {
			width = x;
			DecoderWriteByte(card, 0x115, 0);	// 1:1 scale
			DecoderDelByte(card, 0x114, 0x02);	// Horiz.Filter disable
		} else {	// picture is more than twice the screen width. sigh.
			return 1;
		}
	} else {		// Pan-Scan, fit height to maximum
		DecoderSetByte(card, 0x117, 0x40);	// pan-scan from bitstream
//TODO
		newwidth = (x * vaspy * paspx / (paspy * vaspx));	// width in right aspect ratio
		newheight = y;
		if (newheight <= 288) {	// less then about half the screen size?
			SIF = 1;
			newheight *= 2;
		} else {
			SIF = 0;
		}
		if ((newwidth == 704) || (newwidth == 720))
			width = newwidth;	// standard sizes?
		//newheight=(y*vaspx*paspy/(paspx*vaspy))*width/x;
		factor = newheight * 100 / y;
		printk(KERN_INFO LOGNAME
		       ": Decoder Open: Display size %d x %d, Picture size %d x %d, Demanded size: %d x %d, factor %d\n",
		       width, height, x, y, newwidth, newheight, factor);
		if (aspect == 3) {	// 16:9 Letterbox
			if (SIF) {	// height * 1.5, SIF Letterbox
				if (RMM)
					return 1;	// not supported!
				height = (y * 3) / 2;
				mode = 3;
			} else {	// height * 0.75, 16:9 Letterbox
				height = (y * 3) / 4;
				mode = 8;
			}
		} else if (aspect == 4) {	// 2.21:1 Letterbox
			if (SIF) {	// height * 1
				height = y;
				mode = 5;
			} else {	// height / 2
				height = y / 2;
				mode = 11;
			}
		} else if (aspect == 2) {	// 3:4 aspect ratio
			if (SIF) {
				height = y * 2;
				mode = ((Field && ~RMM) ? 9 : 10);
			} else if (newwidth > 720) {	// picture too wide, scale down to 3/4
				height = (y * 3) / 4;
				mode = 8;
			} else {
				height = y;
				mode = ((Field) ? 7 : 5);
//        mode=((Field)?5:7);
			}
		}
		width = (x * vaspy * paspx / (paspy * vaspx)) * height / y;
		if (x < width) {	// does the picture needs a horizontal blow-up?
			DecoderWriteByte(card, 0x115,
					 ((x * 256 + width - 1) / width) & 0xFF);	// Horiz.Filter scale, x/width*256, rounded up
			DecoderSetByte(card, 0x114, 0x02);	// Horiz.Filter enable
		} else if (x == width) {
			DecoderWriteByte(card, 0x115, 0);	// 1:1 scale
			DecoderDelByte(card, 0x114, 0x02);	// Horiz.Filter disable
		} else if (x <= 720) {
			width = x;
			DecoderWriteByte(card, 0x115, 0);	// 1:1 scale
			DecoderDelByte(card, 0x114, 0x02);	// Horiz.Filter disable
		} else {	// picture is more than twice the screen width. sigh.
			return 1;
		}
	}
	printk(KERN_INFO LOGNAME
	       ": Decoder Open: Display size %d x %d, Picture size %d x %d  Mode: %d\n",
	       width, height, x, y, mode);

	// calculate new picture start- and end rows and columns
	height /= 2;		// convert back to field height
	top += ((bottom - top + 1 - height) / 2);
	if (top < 0)
		top = 0;
	bottom = top + height - 1;
	width *= 2;		// convert back to clocks
	left += ((right - left + 1 - width) / 2);
	if (left < 0)
		left = 0;
	right = left + width - 1;
	DecoderWriteByte(card, 0x12C, left & 0xFF);	// Start- and End Column
	DecoderWriteByte(card, 0x12D, right & 0xFF);
	DecoderWriteByte(card, 0x12E,
			 ((right >> 4) & 0x70) | ((left >> 8) & 0x07));
	DecoderWriteByte(card, 0x129, top & 0xFF);	// Start- and End Row
	DecoderWriteByte(card, 0x12A, bottom & 0xFF);
	DecoderWriteByte(card, 0x12b,
			 ((bottom >> 4) & 0x70) | ((top >> 8) & 0x07));

	DecoderWriteByte(card, 0x116, ((x + 7) / 8) & 0x7F);	// Main Reads per Line

	// set the new mode
	DecoderMaskByte(card, 0x114, 0x78, (mode & 0x0F) << 3);

	MDEBUG(3,": Decoder Open: top/bottom/width / left/right/height  / main reads %d/%d/%d / %d/%d/%d / %d\n",top,bottom,width,left,right,height,((x+7)/8)&0x7F);

	// set the frame store buffers
	if ((i = DecoderSetFrameBuffers(card, y, 0, RMM))) {
		MDEBUG(0,": SetFrameBuffers failed for buffer at 0x%03X\n",i);
		DecoderKillFrameBuffers(card);
		return 2;
	}

	card->lastvattr = 0;
	card->DecoderOpen = 1;
	return 0;
}

// displays a still image, whose pixel data is in luma and chroma
int DecoderShowStill(struct cvdv_cards *card, int width, int height,
		     u8 * luma, u8 * chroma)
{
	u16 addr;
	DecoderOpen(card, width, height,
		    (((width == 320) || (width == 640) || (width == 384)
		      || (width == 768)) ? 1 : 2),
		    ((height < 313) ? 1 : 0), 1, 0);
	addr =
	    ((DecoderReadWord(card, 0x11D) == DecoderReadWord(card, 0x0E0))
	     ? 0x0E4 : 0x0E0);	// choose invisible frame
	DRAMWriteByte(card, DecoderReadWord(card, addr) << 5,
		      width * height, luma, 1);
	DRAMWriteByte(card, DecoderReadWord(card, addr + 2) << 5,
		      width * height / 2, chroma, 1);
	DecoderStillImageDisplay(card, ((height < 313) ? 2 : 1),
				 DecoderReadByte(card, 0x116) & 0x7F,
				 DecoderReadWord(card, addr),
				 DecoderReadWord(card, addr + 2));
	VideoSetBackground(card, 0, 0, 0, 0);	// video on black
	return 0;
}

// TODO: untested, probably won't work (have to use "main reads per line" instead of width on SIF)
int DecoderGetStill(struct cvdv_cards *card, int *width, int *height,
		    u8 * luma, u8 * chroma)
{
	int framebuffer;
	if (card->DecoderOpen) {
		//*width=((DecoderReadByte(card,0x12D)|((DecoderReadByte(card,0x12E)&0x70)<<4))-(DecoderReadByte(card,0x12C)|((DecoderReadByte(card,0x12E)&0x07)<<8))+1)/2;  // screen width, 2 clocks = 1 pixel
		*width = DecoderReadByte(card, 0x116) * 8;
		*height =
		    ((DecoderReadByte
		      (card,
		       0x12A) | ((DecoderReadByte(card, 0x12B) & 0x70) <<
				 4)) -
		     (DecoderReadByte(card, 0x129) |
		      ((DecoderReadByte(card, 0x12B) & 0x07) << 8)) + 1) * 2;	// screen (frame) height
		if ((luma != NULL) && (chroma != NULL)) {
			framebuffer =
			    (((DecoderReadByte(card, 0x0EE) & 0x0C) == 1) ?
			     0x0E4 : 0x0E0);
			DRAMReadByte(card,
				     DecoderReadWord(card,
						     framebuffer) << 5,
				     (*width) * (*height), luma, 1);
			DRAMReadByte(card,
				     DecoderReadWord(card,
						     framebuffer + 2) << 5,
				     (*width) * (*height) / 2, chroma, 1);
		}
		return 0;
	} else
		return 1;
}
