/* 
    cvdvtypes.h

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

       /////////////////////////////////////////////////
      //                                             //
     //  Convergence Digital Video Decoder Card     //
    //  External Definitions for the Char-Driver   //
   //  Copyright (c) 1999 Christian Wolff /       //
  //  convergence integrated media GmbH Berlin   //
 //                                             //
/////////////////////////////////////////////////

// As of 1999-11-09

#ifndef _CVDVTYPE_H_
#define _CVDVTYPE_H_

// our ioctl number: _IOC_TYPE() is 0xA2 (162) and the range of _IOC_NR() is 0x00 to 0x0F.
// submitted 99/10/15 to mec@shout.net
#define CVDV_IOCTL_MAGIC 0xA2

// command numbers _IOC_NR() for ioctl
typedef enum {
	IOCTL_DRAW = 0x01,
	IOCTL_DECODER = 0x02
} IOCTL_Command;


// supported Videosystems
// everything but PAL and NTSC is untested and probably won't work.
typedef enum {
	NTSC = 1,		// NTSC 29.97 fps
	NTSC60,			// NTSC 30 fps
	PAL,			// PAL-B, D, G, H, I, 25 fps
	PALM,			// PAL-M 29.97 fps
	PALM60,			// PAL-M 30 fps
	PALN,			// PAL-N 25 fps
	PALNc,			// PAL-Nc 25 fps
	PAL60			// PAL 30 fps (doesn't work, yet...)
} videosystem;

typedef enum {
	stream_none = 0,	// unknown
	stream_ES,
	stream_PES,
	stream_PS,
	stream_DVD
} stream_type;

typedef enum {
	audio_disable = -1,
	audio_none = 0,		// unknown
	audio_MPEG,
	audio_MPEG_EXT,
	audio_LPCM,
	audio_AC3,
	audio_DTS,
	audio_SDDS
} audio_type;

#if 0
typedef enum {
	// All functions return -2 on "not open"
	OSD_Close = 1,		// ()
	// Disables OSD and releases the buffers
	// returns 0 on success
	OSD_Open,		// (x0,y0,x1,y1,BitPerPixel[2/4/8](color&0x0F),mix[0..15](color&0xF0))
	// Opens OSD with this size and bit depth
	// returns 0 on success, -1 on DRAM allocation error, -2 on "already open"
	OSD_Show,		// ()
	// enables OSD mode
	// returns 0 on success
	OSD_Hide,		// ()
	// disables OSD mode
	// returns 0 on success
	OSD_Clear,		// ()
	// Sets all pixel to color 0
	// returns 0 on success
	OSD_Fill,		// (color)
	// Sets all pixel to color <col>
	// returns 0 on success
	OSD_SetColor,		// (color,R{x0},G{y0},B{x1},opacity{y1})
	// set palette entry <num> to <r,g,b>, <mix> and <trans> apply
	// R,G,B: 0..255
	// R=Red, G=Green, B=Blue
	// opacity=0:      pixel opacity 0% (only video pixel shows)
	// opacity=1..254: pixel opacity as specified in header
	// opacity=255:    pixel opacity 100% (only OSD pixel shows)
	// returns 0 on success, -1 on error
	OSD_SetPalette,		// (firstcolor{color},lastcolor{x0},data)
	// Set a number of entries in the palette
	// sets the entries "firstcolor" through "lastcolor" from the array "data"
	// data has 4 byte for each color:
	// R,G,B, and a opacity value: 0->transparent, 1..254->mix, 255->pixel
	OSD_SetTrans,		// (transparency{color})
	// Sets transparency of mixed pixel (0..15)
	// returns 0 on success
	OSD_SetPixel,		// (x0,y0,color)
	// sets pixel <x>,<y> to color number <col>
	// returns 0 on success, -1 on error
	OSD_GetPixel,		// (x0,y0)
	// returns color number of pixel <x>,<y>,  or -1
	OSD_SetRow,		// (x0,y0,x1,data)
	// fills pixels x0,y through  x1,y with the content of data[]
	// returns 0 on success, -1 on clipping all pixel (no pixel drawn)
	OSD_SetBlock,		// (x0,y0,x1,y1,increment{color},data)
	// fills pixels x0,y0 through  x1,y1 with the content of data[]
	// inc contains the width of one line in the data block,
	// inc<=0 uses blockwidth as linewidth
	// returns 0 on success, -1 on clipping all pixel
	OSD_FillRow,		// (x0,y0,x1,color)
	// fills pixels x0,y through  x1,y with the color <col>
	// returns 0 on success, -1 on clipping all pixel
	OSD_FillBlock,		// (x0,y0,x1,y1,color)
	// fills pixels x0,y0 through  x1,y1 with the color <col>
	// returns 0 on success, -1 on clipping all pixel
	OSD_Line,		// (x0,y0,x1,y1,color)
	// draw a line from x0,y0 to x1,y1 with the color <col>
	// returns 0 on success
	OSD_Query,		// (x0,y0,x1,y1,xasp{color}}), yasp=11
	// fills parameters with the picture dimensions and the pixel aspect ratio
	// returns 0 on success
	OSD_Test		// ()
	    // draws a test picture. for debugging purposes only
	    // returns 0 on success
// TODO: remove "test" in final version
} OSD_Command;


struct drawcmd {
	OSD_Command cmd;
	int x0;
	int y0;
	int x1;
	int y1;
	int color;
	void *data;
};
#endif

typedef enum {
	Decoder_Pause,		// pause{param1}  0=run 1=pause 2=toggle
	Decoder_Still_Put,	// (width{param1}, height{param2}, luma{data1}, chroma{data2})
	// show still picture of specified size
	// width;    width of the image
	// height;   height of the image
	// luma;     Y values, one byte per pixel, width*height bytes
	// chroma;   4:2:0 U and V values, interlaced, one byte each U, one byte each V, width*height/2 bytes
	Decoder_Still_Get,	// (width{param1}, height{param2}, luma{data1}, chroma{data2})
	// grab current showing image
	// width and height will be set to current picture size
	// if luma and croma are NULL, only width and height will be reported
	// otherwise the pixel data is filled in there, same format as Still_put
	Decoder_Set_Videosystem,	// (videosystem{param1})
	// videosystem: see enum {} videosystem;
	Decoder_Set_Streamtype,	// (streamtype{param1})
	// streamtype: according to enum {} stream_type;
	// This has to be set BEFORE you send data to the device
	// For ES and PES streams, Audio has to go into the first device (e.g.minor 0) and video into the second (e.g.minor 16)
	Decoder_Set_Audiotype,	// (audiotype{param1})
	// audiotype: see enum {} audio_type, +16 for IEC956 on S/PDIF out
	Decoder_Set_VideoStreamID,	// (video stream ID {param1})
	// video stream ID: MPEG ID 0..15 of video stream to display (E0..EF), -1 for any/auto
	Decoder_Set_AudioStreamID,	// (audio stream ID {param1}, audio extension stream ID {param2})
	// audio stream ID: MPEG ID 0..31 of audio stream to display (C0..DF), -1 for any/auto
	// audio extension stream ID: MPEG ID 0..31 of audio extension stream (C0..DF), -1 for none
	Decoder_CSS,		// Passes CSS information to and from the decoder
	// action{param1},
	// data block{data1} MSB first
	// execute 1 to 4 once for each disc, then 5 to 8 for each title
	// returns 0 on success, <0 on error
	//   -1: timeout reading data from card
	//   -2: data pointer not initialized
	//   -3: invalid action number
	// action=0 -> disable and bypass CSS
	// Disk key:
	// action=1 -> retreive drive challenge (10 byte) from card
	// action=2 -> post drive response (5 byte) to card
	// action=3 -> post card challenge (10 byte) and retreive card response (5 byte)
	// action=4 -> post disk key (2048 byte) into the card
	// Title key:
	// action=5 -> retreive title challenge (10 byte) from card
	// action=6 -> post title response (5 byte) to card
	// action=7 -> post card challenge (10 byte) and retreive card response (5 byte)
	// action=8 -> post encrypted title key (5 byte) into the card
	Decoder_Highlight,	// post SPU Highlight information,
	// active{param1}
	//   1=show highlight, 0=hide highlight
	// color information(SL_COLI or AC_COLI){data1[4]} MSB first 
	//   bits:  descr.
	//   31-28  Emphasis pixel-2 color
	//   27-24  Emphasis pixel-1 color
	//   23-20  Pattern pixel color
	//   19-16  Background pixel color
	//   15-12  Emphasis pixel-2 contrast
	//   11- 8  Emphasis pixel-1 contrast
	//    7- 4  Pattern pixel contrast
	//    3- 0  Background pixel contrast
	// button position(BTN_POSI){data2[6]} MSB first
	//   bits:  descr.
	//   47-46  button color number
	//   45-36  start x
	//   33-24  end x
	//   23-22  auto action mode
	//   21-12  start y
	//    9- 0  end y
	Decoder_SPU,		// Activate SPU decoding and select SPU stream ID
	// stream{param1}
	// active{param2}
	Decoder_SPU_Palette,	// post SPU Palette information
	// length{param1}
	// palette{data1}
	Decoder_GetNavi,	// Retreives CSS-decrypted navigational information from the stream.
	// data1 will be filled with PCI or DSI pack (private stream 2 stream_id), 
	// and the length of data1 (1024 or 0) will be returned
	Decoder_SetKaraoke,	// Vocal1{param1}, Vocal2{param2}, Melody{param3} 
	// if Vocal1 or Vocal2 are non-zero, they get mixed into left and right at 70% each
	// if both, Vocal1 and Vocal2 are non-zero, Vocal1 gets mixed into the left channel and
	// Vocal2 into the right channel at 100% each.
	// if Melody is non-zero, the melody channel gets mixed into left and right
	Decoder_Set_Videoattribute,	// Set the video parameters
	// attribute{param1} (2 byte V_ATR)
	//   bits: descr.
	//   15-14 Video compression mode (0=MPEG-1, 1=MPEG-2)
	//   13-12 TV system (0=525/60, 1=625/50)
	//   11-10 Aspect ratio (0=4:3, 3=16:9)
	//    9- 8 permitted display mode on 4:3 monitor (0=both, 1=only pan-scan, 2=only letterbox)
	//    7    line 21-1 data present in GOP (1=yes, 0=no)
	//    6    line 21-2 data present in GOP (1=yes, 0=no)
	//    5- 3 source resolution (0=720x480/576, 1=704x480/576, 2=352x480/576, 3=352x240/288)
	//    2    source letterboxed (1=yes, 0=no)
	//    0    film/camera mode (0=camera, 1=film (625/50 only))
	Decoder_Set_Audioattribute,	// Set the audio parameters
	// attribute{param1} (2 most significan bytes of A_ATR (bit 63 through 48))
	//   bits: descr.
	//   15-13 audio coding mode (0=ac3, 2=mpeg1, 3=mpeg2ext, 4=LPCM, 6=DTS, 7=SDDS)
	//   12    multichannel extension
	//   11-10 audio type (0=not spec, 1=language included)
	//    9- 8 audio application mode (0=not spec, 1=karaoke, 2=surround)
	//    7- 6 Quantization / DRC (mpeg audio: 1=DRC exists)(lpcm: 0=16bit, 1=20bit, 2=24bit)
	//    5- 4 Sample frequency fs (0=48kHz, 1=96kHz)
	//    2- 0 number of audio channels (n+1 channels)
	Decoder_WriteBlock,	// Post one block of data, e.g. one DVD sector of 2048 byte, into the decoder queue
	    // sectordata{data1}
	    // length{param1}
	    // is_initial_block{param2}
	    // set_SCR{param3}
	/*
	Decoder_FFWD,		// ffwd{param1}  0=normal 1=ffwd 2=toggle
	Decoder_Slow		// slow{param1}  0=normal 1=slow 2=toggle
	*/
} Decoder_Command;

struct decodercmd {
	Decoder_Command cmd;
	int param1;
	int param2;
	int param3;
	int param4;
	int param5;
	void *data1;
	void *data2;
};

#endif				// _CVDVTYPE_H_
