/* 
    video.h

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

#ifndef CVDV_VIDEO_H
#define CVDV_VIDEO_H

  //
 //  Video Decoder
//

#include "cardbase.h"

// Set the background of the OSD and SPU and it's color
// mode=0: Video on Black
// mode=1: Black
// mode=2: Selected Color
// mode=3: Video on Selected Color
void VideoSetBackground(struct cvdv_cards *card, int mode, u8 Y, u8 Cb,
			u8 Cr);


int DecoderStartDecode(struct cvdv_cards *card);

int DecoderStopDecode(struct cvdv_cards *card);

// Sets Display Override (Still Image Display) to Frame Buffer at specified addresses,
// addresses are 16 bit, in 64 byte resolution
// mode: 0=off, 1=Frame, 2=Field
// width: width of the still picture in 8 pixel units
int DecoderStillImageDisplay(struct cvdv_cards *card, int mode, int width,
			     u16 LumaAddr, u16 ChromaAddr);

// Frees allocated frame buffers
int DecoderKillFrameBuffers(struct cvdv_cards *card);

int DecoderSetFrameBuffers(struct cvdv_cards *card, int lines,	// number of lines of the decoded MPEG
			   int TwoFrames,	// 1 if no B-Frames are present in the video stream, thus allowing only 2 framestores
			   int RMM);	// 1 if RMM

// returns size of the Video ES Buffer in bytes or 0=error
u32 DecoderGetVideoESSize(struct cvdv_cards *card);

// returns level of fullness in bytes
u32 DecoderGetVideoESLevel(struct cvdv_cards *card);

// pics=0 --> items=bytes
// pics=1 --> items=pictures
void DecoderSetVideoPanic(struct cvdv_cards *card, int pics, int items);

int DecoderClose(struct cvdv_cards *card);

// returns 0 on success, 1 on "picture size too big", 2 on "out of DRAM memory"
int DecoderOpen(struct cvdv_cards *card, int x, int y,	// size of the decoded MPEG picture
		int aspect,	// pixel or picture aspect ratio of the MPEG picture: 1=square pixel 2=3:4 3=9:16 4=1:2.21
		int Field,	// 0:Frame (interlaced, MPEG-2) , 1:Field (non-interlaced, MPEG-1) structure
		int Letterbox,	// 0:PanScan (4:3), 1:letterbox (16:9, 8:3) picture ratio  // TODO, ignored for now
		int RMM		// 1:use ReducedMemoryMode
    );

// displays a still image, whose pixel data is in luma and chroma
int DecoderShowStill(struct cvdv_cards *card, int width, int height,
		     u8 * luma, u8 * chroma);

// TODO: untested, probably won't work (have to use "main reads per line" instead of width on SIF)
int DecoderGetStill(struct cvdv_cards *card, int *width, int *height,
		    u8 * luma, u8 * chroma);

#endif				/* CVDV_VIDEO_H */
